/* Handle a single node transaction queue, keep loading the transactions into a transaction pool,
 * as node_proc is data status is rappresented via the ADT structure "node".
 *
 * -> responsability: is responsable of mantaining the tranzaction pool and updating the node_tp_shm
 *                    and advice the user in case of max transaction in tp by sending back the
 *                    failure. Also sending an advice to the proc_master
 *                    of his termination in case of failure or timeout.
 * */

/*  Standard Library */
#include <stdio.h>
#include <stdlib.h>

/* Sys  */
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/shm.h>

/*  General */
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>


/*  Local Library  */
#include "local_lib/headers/glob.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/simulation_errors.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/node_transactor.h"
#include "local_lib/headers/user_msg_report.h"
#include "local_lib/headers/node_msg_report.h"
#include "local_lib/headers/conf_shm.h"
#include "local_lib/headers/book_master_shm.h"
#include "local_lib/headers/master_msg_report.h"
#include "local_lib/headers/node_tp_shm.h"
#ifdef DEBUG

#ifdef DEBUG_NODE_TP
#include "local_lib/headers/debug_utility.h"
#else
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif
#endif

/* Support Function*/
/**
 * \brief Acquire the semaphore_id related to the node
 * In particular:
 * - semaphore_start_id
 * - semaphore_node_tp_shm_access
 */
void acquire_semaphore_ids(void);
/**
 * Check the argc and argv to match with project specification and load so_tp_size
 * @param argc number of argument given
 * @param argv pointer to a char list of params given
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool check_arguments(int argc, char const *argv[]);

/**
 * Connects to the differents queues: master, node's and user's
 * */
void connect_to_queues(void);

/*
 * Load the block into the shm_node_tp if semval is set to 0
 * */
void load_block_to_shm(void);

/**
 * handler of the signal
 * @param signum type of signal to be handled
 */
void signals_handler(int signum);
/* TODO: refactoring of comments of the following process transaction*/
/**
 * If it finds node msg of type NODE_TRANSACTION it proccess them
 * and make the aknowledgement
 * @param msg_rep the messagge to be loaded if present in the queue
 * */
void process_node_transaction(struct node_msg *msg_rep);

/*
 * If it finds node msg of type TRANSACTION_TYPE it process them
 * and make the aknowledgement adding them to the transaction_pool and the transaction_block if
 * need
 * @param msg_rep the messagge to be loaded if present in the queue
 * */
void process_simple_transaction_type(struct node_msg *msg_rep);

/*
 * Get new SO_BLOCK_SIZE transactions from the transaction_list in the current_node_tp
 * */
void update_block(void);

/* SysVar */
int sem_start_id = -1; /*Id of the start semaphore arrays for sinc*/
int sem_node_tp_id = -1; /* Id of the semaphore for accesing node_tp_shm*/
int queue_node_id = -1;/* Identifier of the node queue id */
int queue_user_id = -1; /* Identifier of the user queue id*/
int node_tp_end = 0; /* For values different from 0 the node proc must end*/
int last_signal;
struct node current_node_tp; /* Current rappresentation of the node_tp */
struct node_block * shm_node_tp; /* Ref to the shm for the node_tp_shm */

int main(int argc, char const *argv[]){
    DEBUG_MESSAGE("NODE TP PROCESS STARTED");
    struct sigaction sa;
    sigset_t sigmask;
    DEBUG_MESSAGE("NODE TP PROCESS SET TO INIT");
    current_node_tp.exec_state = PROC_STATE_INIT;
    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
    if(check_arguments(argc,argv)==TRUE){
        struct node_msg msg_rep;
        node_create(&current_node_tp,getpid(),0, current_node_tp.type.tp.tp_size, SO_BLOCK_SIZE, 0, &calc_reward);
        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/
        /*---------------------------*/
        /*  SEMAPHORES ACQUISITION   */
        /*---------------------------*/
        acquire_semaphore_ids();

        /*-----------------------*/
        /*  CONNECTING TO QUEUES */
        /*-----------------------*/
        connect_to_queues();

        /*---------------------------*/
        /*  SEMAPHORES SINC..        */
        /*---------------------------*/
        DEBUG_MESSAGE("NODE READY, ON START_SEM");
        if (semaphore_wait_for_sinc(sem_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO WAIT FOR START");
        }
        current_node_tp.exec_state= PROC_STATE_RUNNING;
        while(node_tp_end != 1){
            process_node_transaction(&msg_rep);
            process_simple_transaction_type(&msg_rep);
            load_block_to_shm();
        }
    }
}


Bool check_arguments(int argc, char const *argv[]) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
    if (argc < 2) {
        ERROR_EXIT_SEQUENCE_NODE_TP("MISSING ARGUMENT");
    }
    int tp_size = atoi(argv[1]);
    int node_id = atoi(argv[2]);
    if(tp_size <=0){
        ERROR_EXIT_SEQUENCE_NODE_TP("TP_SIZE IS <= 0. NOT ACCEPTED");
    }
    current_node_tp.type.tp.tp_size=tp_size;
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}
void acquire_semaphore_ids(void){
    sem_start_id= semget(SEMAPHORE_SINC_KEY_START, 1, 0);
    if (sem_start_id< 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN ID OF START SEM"); }
    if (semaphore_lock(sem_start_id, 0) < 0) {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
    }
    sem_node_tp_id  = semget(SEMAPHORE_TP_SHM_KEY, 1, 0);
    if(sem_node_tp_id < 0) {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO RETRIEVE");
    }
}
void process_simple_transaction_type(struct node_msg *msg_rep) {
    if (node_msg_receive(queue_node_id, msg_rep, current_node_tp.node_id) == 0) {
        DEBUG_MESSAGE("NODE TRANSACTION TYPE RECEIVED");
        if (get_num_transactions(current_node_tp.transactions_list) < current_node_tp.type.tp.tp_size) {
            queue_append(current_node_tp.transactions_list, msg_rep->t);
        } else {/*TP_SIZE FULL*/
            struct user_msg *u_msg_rep;
            DEBUG_ERROR_MESSAGE("NODE TRANSACTION FAILED");
            u_msg_rep->t.t_type = TRANSACTION_FAILED;
            user_msg_snd(queue_user_id, u_msg_rep, MSG_TRANSACTION_FAILED_TYPE, &msg_rep->t, current_node_tp.pid, TRUE);
        }
    }
}
void process_node_transaction(struct node_msg *msg_rep) {
    if (node_msg_receive(queue_node_id, msg_rep, current_node_tp.node_id-MSG_NODE_ORIGIN_TYPE) == 0) {
        /*Checking for transaction coming from node*/
        DEBUG_MESSAGE("NODE TRANSACTION RECEIVED");
        /*TODO: Implement incoming transaction from other node*/
    }
}
void load_block_to_shm(void){
    int sem_val = semctl(sem_node_tp_id, 0, GETVAL);
    if(sem_val == IS_EMPTY && get_num_transactions(current_node_tp.transactions_list) >= SO_BLOCK_SIZE){
        update_block();
        if (semctl(sem_node_tp_id,0,SETVAL, FULL) < 0){
            ERROR_MESSAGE("IMPOSSIBLE TO SET NODE TP SEM TO FULL");
        }
    }else if (sem_val < 0){
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIVE VAL FROM TP SHM SEMAPHORE");
    }
}
void update_block(void){
    int i;
    for (i=0; i < SO_BLOCK_SIZE; i++){
        shm_node_tp->block_t[i] = queue_head(current_node_tp.transactions_list);
        queue_remove_head(current_node_tp.transactions_list);
    }
}
void signals_handler(int signum) {
    DEBUG_SIGNAL("SIGNAL RECEIVED ", signum);
    last_signal = signum;
    switch (signum) {
        case SIGINT:
            /*TODO: avvisare main*/
            alarm(0);/*pending alarm removed*/
            EXIT_PROCEDURE_NODE_TP(0);
        case SIGALRM:
            break;
        default:
            break;
    }
}
void connect_to_queues(void) {
    queue_node_id = msgget(NODES_QUEUE_KEY, 0600);
    if (queue_node_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO NODE MESSAGE QUEUE"); }
    queue_user_id = msgget(USERS_QUEUE_KEY, 0600);
    if (queue_user_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO USER QUEUE"); }
}

void free_mem_node_tp(){
    free_node(&current_node_tp);
}
void free_sysVar_node_tp(){
    int semaphore_start_value;
    /* If the process is in INIT STATE his termination can block the whole simulation. So it need to
     * lock the sart semaphore.
     * If the semaphore_start_id is impossible to retrive ( = -1) nothing can be done.
     * */
    if (current_node_tp.exec_state== PROC_STATE_INIT && sem_start_id>= 0) {
        semaphore_start_value = semctl(sem_start_id, 0, GETVAL);
        if (semaphore_start_value < 0) ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0 && (semaphore_lock(sem_start_id, 0) < 0)) {
            ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }
}
