#define _GNU_SOURCE
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
#include "local_lib/headers/debug_utility.h"


/* Support Function*/
/**
 * Advice the users of the processed transactions, both receivers and senders
 */
void adv_users_of_block(void);

/**
 * handler of the signal
 * @param signum type of signal to be handled
 */
void signals_handler(int signum);

/**
 * \brief Read the conf file present in te project dir
 * load the configuration directly in the struct conf node_configuration that is a SysVar
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool read_conf_node();

/**
 * Set the handler for signals of the current node_proc
 * @param sa describe the type of action to be performed when a signal arrive
 * @param sigmask the mask to be applied
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool set_signal_handler_node(struct sigaction sa, sigset_t sigmask);

/**
 * Check the argc and argv to match with project specification
 * @param argc number of argument given
 * @param argv pointer to a char list of params given
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool check_arguments(int argc, char const *argv[]);

/**
 * Create and Lunch the node transaction pool process
 */
void create_node_tp_proc(void);

/**
 * \brief Attach the current node to the related shms
 * Specificcaly:
 *  -book_master_shm
 *  -node_tp_shm
 */
void attach_to_shms(void);

/**
 * If node_transaction pool has at list node_block_size of transactions it load them in a transaction block
 * and process them
 * @return -1 in case of failure. 2 in case of pool_size<block_size. 0 otherwise
 */
int process_node_block();

/**
 * Connects to the differents queues: master, node's and user's
 * */
void connect_to_queues(void);

/**
 * load block_size transactions from transaction pool into transaction block of the current node
 * @return TRUE if succeded, FALSE otherwise
 */
Bool load_block(void);

/**
 * \brief Acquire the semaphore_id related to the node
 * Specificcaly:
 * - semaphore_start_id
 * - semaphore_masterbook_id
 * - semaphore_node_tp_shm_access
 */
void acquire_semaphore_ids(void);

/* Lock the shm book_master via masterbook_to_fill param and the cell to fill index.
 * @return FALSE in case of failure, TRUE otherwise
 */
Bool lock_shm_masterbook(void);

/* Lock the to_fill index saved in the shared_memory access
 */
void lock_to_fill_sem(void);

/* Lock the current cell of the masterbook shm in which the node had written the block
 * @param i_cell_block_list index to be blocked
 */
void lock_masterbook_cell_access(unsigned short int i_cell_block_list);

/* Unlock the to_fill index saved in the shared_memory access
 */
void unlock_to_fill_sem(void);

/* Unloack the current cell of the masterbook shm in which the node had written the block
 */
void unlock_masterbook_cell_access(void);

/* Advice the master porc via master_message queue by sending a master_message
 * @param termination_type MSG_REPORT_TYPE type process termination occured
*/
void advice_master_of_termination(int termination_type);

/*Extract randomly the time needed for processing from node_configuration
 *@return an integer that rappresent the time for processing
 * */
long int get_time_processing(void);

/*
 * Create the semaphores needed for the access regulation for this specific program
 */
void create_semaphore(void);

/*
 * Create the shmared memory for the tp (transaction pool) of the current node
 * */
void create_tp_shm(void);

/* SysVar */
int shm_tp_id = -1; /* Id of node_tp_shm*/
int semaphore_start_id = -1; /*Id of the start semaphore arrays for sinc*/
int semaphore_masterbook_id = -1; /*Id of the masterbook semaphore for accessing the block matrix*/
int semaphore_to_fill_id = -1; /* Id of the masterbook to_fill access semaphore*/
int semaphore_tp_shm = -1; /* Id of the tp_shm for accesing the current block to be processed*/
int queue_node_id = -1;/* Identifier of the node queue id */
int queue_user_id = -1; /* Identifier of the user queue id*/
int queue_master_id = -1; /* Identifier of the master queue id*/
int node_end = 0; /* For values different from 0 the node proc must end*/
float current_block_reward = 0; /* The current value of all node block reward*/
int last_signal;
struct node current_node; /* Current representation of the node*/
struct conf node_configuration; /* Configuration File representation*/
struct shm_conf *shm_conf_pointer_node; /* Ref to the shm for configuration of the node*/
struct shm_book_master *shm_masterbook_pointer;/* Ref to the shm for the masterbook shm */
struct node_block * shm_node_tp; /* Ref to the shm for the node_tp shm */

int main(int argc, char const *argv[]) {
    DEBUG_MESSAGE("NODE PROCESS STARTED");
    struct sigaction sa;
    sigset_t sigmask;
    DEBUG_MESSAGE("NODE STATE SET TO INIT");
    current_node.exec_state= PROC_STATE_INIT;
    current_node.pid = getpid();
    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
    read_conf_node(&node_configuration);
    node_proc_block_create(&current_node, getpid(), 0, SO_BLOCK_SIZE,
            node_configuration.so_reward, (Reward)&calc_reward);
    if (check_arguments(argc, argv) && set_signal_handler_node(sa, sigmask)) {
        /*-----------------------*/
        /*  CREATING SEMAPHORES  */
        /*-----------------------*/
        create_semaphore();
        /*-----------------------*/
        /*  CREATING SHM TP      */
        /*-----------------------*/
        create_tp_shm();
        /*-----------------------*/
        /*  CONNECTING TO QUEUES */
        /*-----------------------*/
        connect_to_queues();
        /*-------------------------*/
        /*  SHARED MEM  CONFIG     */
        /*-------------------------*/
        attach_to_shms();
        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/
        /*---------------------------*/
        /*  SEMAPHORES ACQUISITION   */
        /*---------------------------*/
        acquire_semaphore_ids();

        /************************************
         *      CREATING NODE_TP_PROC       *
         * **********************************/
        create_node_tp_proc();

        /*---------------------------*/
        /*  SEMAPHORES SINC..        */
        /*---------------------------*/
        DEBUG_MESSAGE("NODE READY, ON START_SEM");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO WAIT FOR START");
        }
        current_node.exec_state= PROC_STATE_RUNNING;

        /****************************************
         *      PROCESSING OF TRANSACTION FASE  *
         * **************************************/
        while (node_end != 1) {
            process_node_block();/*Implement the load from the shm*/
        }
    }
    advice_master_of_termination(TERMINATION_END_CORRECTLY);
    EXIT_PROCEDURE_NODE(0);
}

Bool read_conf_node() {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION...");
    switch (load_configuration(&node_configuration)) {
        case 0:
            break;
        case -1:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: MISSING FILE OR EMPTY");
        case -2:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: BROKEN SIMULTATION LOGIC, CHECK CONF. VALUE");
        case -3:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: NOT ENOUGH USERS FOR NODES");
        case -4:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: MIN MAX EXECUTION TIME WRONG");
        case -5:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: NODE REWARD IS OVER POSSIBILITIES OF USERS");
        default:
            return FALSE;
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CONFIGURATION LOADED");
    return TRUE;
}

Bool check_arguments(int argc, char const *argv[]) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
    if (argc < 2) {
        ERROR_EXIT_SEQUENCE_NODE("MISSING ARGUMENT");
    }
    current_node.node_id = atoi(argv[1]);
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}

Bool set_signal_handler_node(struct sigaction sa, sigset_t sigmask) {
/*    DEBUG_NOTIFY_ACTIVITY_RUNNING("SETTING SIGNAL MASK...");
    sigemptyset(&sigmask);/*Creating an empty mask*/
   /* sigaddset(&sigmask, SIGALRM);/*Adding signal to the mask*/
    /*DEBUG_NOTIFY_ACTIVITY_DONE("SETTING SIGNAL MASK DONE");*/

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signals_handler;
    sa.sa_mask = sigmask;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0 ||
        sigaction(SIGUSR2, &sa, NULL) < 0
            ) {
        ERROR_EXIT_SEQUENCE_NODE("ERROR DURING THE CREATION OF THE SIG HANDLER ");
    }
    return TRUE;
}

void signals_handler(int signum) {
    DEBUG_SIGNAL("SIGNAL RECEIVED ", signum);
    last_signal = signum;
    struct master_msg_report msg;
    switch (signum) {
        case SIGINT:
            alarm(0);/*pending alarm removed*/
            current_node.exec_state = PROC_STATE_TERMINATED;
            advice_master_of_termination(SIGNALS_OF_TERM_RECEIVED);
            EXIT_PROCEDURE_NODE(0);
            break;
        case SIGALRM:
            break;
        case SIGUSR2:
            master_msg_send(queue_master_id,&msg, INFO_BUDGET,NODE,current_node.pid,current_node.exec_state,TRUE, current_node.type.block.budget);
            break;
        default:
            break;
    }
}

void free_sysVar_node() {
    int semaphore_start_value;
    /* If the process is in INIT STATE his termination can block the whole simulation. So it need to
     * lock the sart semaphore.
     * If the semaphore_start_id is impossible to retrive ( = -1) nothing can be done.
     * */
    if (current_node.exec_state== PROC_STATE_INIT && semaphore_start_id >= 0) {
        semaphore_start_value = semctl(semaphore_start_id, 0, GETVAL);
        if (semaphore_start_value < 0) ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0 && (semaphore_lock(semaphore_start_id, 0) < 0)) {
            ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING SEMAPHORE_TP_SHM...");
    if(semaphore_tp_shm >= 0 && semctl(semaphore_tp_shm,0,IPC_RMID) < 0){
        ERROR_MESSAGE("REMOVING PROCEDURE FOR TP SHM SEMAPHORE HAS FAILED");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING SEMAPHORE_TP_SHM DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING TP_SHM ....");
    if(shm_tp_id>= 0 && shmctl(shm_tp_id, IPC_RMID, NULL) < 0){
        ERROR_MESSAGE("REMOVING PROCEDURE FOR TP SHM HAS FAILED");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING TP_SHM DONE");
    current_node.exec_state = PROC_STATE_TERMINATED;
}

void free_mem_node() {
    free_node(&current_node);
}

void attach_to_shms(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("ATTACHING TO SHM...");
    int shm_conf_id = -1;/* id to the shm_conf*/
    shm_conf_id = shmget(SHM_CONFIGURATION, sizeof(struct shm_conf), 0600);
    if (shm_conf_id < 0) {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO ACCESS SHM CONF");
    }
    shm_conf_pointer_node = shmat(shm_conf_id, NULL, 0);
    if (shm_conf_pointer_node == (void *) -1) {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO SHM CONF");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO SHM DONE");
    shm_conf_id = shmget(MASTER_BOOK_SHM_KEY, sizeof(struct shm_book_master), 0600);
    if (shm_conf_id<0){
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO ACCESS SHM BOOKMASTER");
    }
    shm_masterbook_pointer = shmat(shm_conf_id, NULL, 0);
    if (shm_masterbook_pointer == (void *)-1){
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO THE SHM_MASTERBOOK");
    }
    shm_node_tp = shmat(shm_tp_id, NULL, 0);
    if(shm_node_tp == (void * )-1){
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO THE NODE_TP SHM");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO SHM DONE");
}


void connect_to_queues(void) {
    queue_node_id = msgget(NODES_QUEUE_KEY, 0600);
    if (queue_node_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO NODE MESSAGE QUEUE"); }
    queue_user_id = msgget(USERS_QUEUE_KEY, 0600);
    if (queue_user_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO USER QUEUE"); }
    queue_master_id = msgget(MASTER_QUEUE_KEY, 0600);
    if (queue_master_id < 0){ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO MASTER QUEUE");}
}

int process_node_block() {
    /*Loading them into the node_block_transactions*/
    if (load_block() == FALSE) return -1;
    if (get_num_transactions(current_node.transactions_list)==SO_BLOCK_SIZE /* DID i got the correct num of transactions*/
            && current_node.type.block.calc_reward(&current_node, -1, TRUE, &current_block_reward)>=0){
        int num_of_shm_retry = 0;
        while(num_of_shm_retry < MAX_FAILURE_SHM_BOOKMASTER_LOCKING
                && lock_shm_masterbook() == FALSE){
            num_of_shm_retry++;
        }
        /*TODO: send confirmed to all users*/
    }

    return 0;
}
Bool lock_shm_masterbook(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("NODE:= LOCKING THE SHM FOR ADDING THE BLOCK...");
    struct timespec trans_proc_sim;
    trans_proc_sim.tv_sec = 0;
    trans_proc_sim.tv_nsec = get_time_processing();
    nanosleep(&trans_proc_sim, (void *) NULL);
    lock_to_fill_sem();
    unsigned short int i_cell_block_list = shm_masterbook_pointer->to_fill;
    /*Inserting the block into the shm*/
    shm_masterbook_pointer->to_fill += 1;
    lock_masterbook_cell_access(i_cell_block_list);
    struct Transaction block_list [get_num_transactions(current_node.transactions_list)];
    queue_to_array(current_node.transactions_list,block_list);
    if (insert_block(shm_masterbook_pointer,block_list) == 0){
        current_node.type.block.budget = current_block_reward;
    }else {
        ERROR_MESSAGE("IMPOSSIBLE TO INSERT BLOCK");
        return FALSE;
    }
    /*Unloacking the semaphore*/
    unlock_masterbook_cell_access();
    unlock_to_fill_sem();
    DEBUG_NOTIFY_ACTIVITY_DONE("NODE:= LOCKING THE SHM FOR ADDING THE BLOCK DONE");
    return TRUE;
}

void lock_to_fill_sem(void) {
    while (semaphore_lock(semaphore_to_fill_id, 0) < 0) {
        if (errno == EINTR) {
            if (last_signal == SIGALRM) {
                /**RICEZIONE DI SEGNALE*/
                unlock_to_fill_sem();
                /*Avvisare il main e il processo deve terminare*/
                advice_master_of_termination(IMPOSSIBLE_TO_SEND_TRANSACTION);
            } else {
                continue;
            }
        } else {
            ERROR_EXIT_SEQUENCE_NODE("ERROR WHILE TRYING TO EXEC LOCK ON TO_FILL ACCESS SEM");
        }
    }
}

void lock_masterbook_cell_access(unsigned short int i_cell_block_list) {
    while (semaphore_lock(semaphore_masterbook_id, i_cell_block_list)) {
        /*TODO: fare refactoring nei due while*/
        if (errno == EINTR) {
            if (last_signal == SIGALRM) {
                /**RICEZIONE DI SEGNALE*/
                unlock_masterbook_cell_access();
                /*Avvisare il main e il processo deve terminare*/
                advice_master_of_termination(IMPOSSIBLE_TO_SEND_TRANSACTION);
            } else {
                continue;
            }
        } else {
            ERROR_EXIT_SEQUENCE_NODE("ERROR WHILE TRYING TO EXEC LOCK ON TO_FILL ACCESS SEM");
        }
    }
}

void unlock_to_fill_sem(void) {
    while (semaphore_unlock(semaphore_to_fill_id, 0)) {
        if (errno != EINTR) {
            ERROR_EXIT_SEQUENCE_NODE("ERROR DURING THE UNLOCK OF THE SEMAPHORE");
        }
    }
}

void unlock_masterbook_cell_access(void) {
    while (semaphore_unlock(semaphore_masterbook_id, 0)) {
        if (errno != EINTR) {
            ERROR_EXIT_SEQUENCE_NODE("ERROR DURING THE UNLOCK OF BOOKMASTER CELL");
        }
    }
}

Bool load_block(void) {
    int sem_val = semctl(semaphore_tp_shm,0, GETVAL);
    if (sem_val==FULL){
        struct Transaction *vector;
        vector = shm_node_tp->block_t;
        if (array_to_queue(current_node.transactions_list, vector)){
            ERROR_MESSAGE("NULL TRANSACTION VECTOR");
        }
        if(semctl(semaphore_tp_shm,0,SETVAL, IS_EMPTY) < 0){
            ERROR_MESSAGE("IMPOSSIBLE TO SET NODE TP SEM TO IS_EMPTY");
            return FALSE;
        }
    }else if (sem_val != IS_EMPTY){
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIVE VAL FROM TP SHM SEMAPHORE");
        return FALSE;
    }
    return TRUE;
}


void advice_master_of_termination(int termination_type) {
    struct master_msg_report termination_report;
    if (master_msg_send(queue_master_id, &termination_report, termination_type, NODE,current_node.pid, current_node.exec_state,TRUE, current_node.type.block.budget)<0){
        ERROR_MESSAGE("IMPOSSIBLE TO ADVICE MASTER OF TERMINATION");
    }
}

void acquire_semaphore_ids(void) {
    semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
    if (semaphore_start_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN ID OF START SEM"); }
    if (semaphore_lock(semaphore_start_id, 0) < 0) {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
    }
    semaphore_masterbook_id = semget(SEMAPHORE_MASTER_BOOK_ACCESS_KEY, 1, 0);
    if (semaphore_masterbook_id < 0) {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN THE MASTERBOOK SEM");
    }
    semaphore_to_fill_id = semget(SEMAPHORE_MASTER_BOOK_TO_FILL_KEY, 1, 0);
    if (semaphore_to_fill_id < 0) {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN MASTERBOOK_TO_FILL SEM");
    }
}

long int get_time_processing(void) {
    srand(getpid());
    return (rand() %
            (node_configuration.so_max_trans_gen_nsec - node_configuration.so_min_trans_proc_nsec + 1)) +
           node_configuration.so_min_trans_proc_nsec;
}
void create_semaphore(void){
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF THE TP_SHM SEM...");
    semaphore_tp_shm = semget(current_node.node_id,1, IPC_CREAT | IPC_EXCL | 0600);
    if(semaphore_tp_shm < 0){
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CREATE TP_SHM SEM");
    }
    if (semctl(semaphore_tp_shm, 0, SETVAL, IS_EMPTY)<0){
       ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO SET NODE TP SEM TO IS_EMPTY");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF THE TP_SHM SEM DONE");
}
void create_tp_shm(void){
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF THE TP_SHM...");
    shm_tp_id = shmget(current_node.pid+current_node.node_id, sizeof(struct node_block), IPC_CREAT | IPC_EXCL | 0600);
    if (shm_tp_id < 0){
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CREATE NODE TP_SHM");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF THE TP_SHM DONE");
}
void free_kids_node(){
    DEBUG_NOTIFY_ACTIVITY_RUNNING("KILLING NODE_TP_PROC RELATED...");
    if (kill(current_node.type.block.kid_pid, SIGINT)>=0 || errno == ESRCH){
        DEBUG_MESSAGE("PROC KILLED");
        DEBUG_NOTIFY_ACTIVITY_DONE("KILLING NODE_TP_PROC DONE");
    }else{
        if(errno == EINTR){return;}
        ERROR_MESSAGE("IMPOSSIBLE TO SEND TERMINATION SIGNAL TO NODE_TP KID");
    }
}
void create_node_tp_proc(void){
   char *argv_node_tp[]={PATH_TO_NODE_TP,NULL,NULL,NULL, NULL};
   pid_t node_tp_pid;
   argv_node_tp[1] = (char *) malloc(11 * sizeof(char));
   argv_node_tp[2] = (char *) malloc(11 * sizeof(char));
   argv_node_tp[3] = (char *) malloc(11 * sizeof(char));
   switch(node_tp_pid =fork()){
       case -1:
           ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO GENERATE NODE_TP");
           break;
       case 0: /*kid*/
           sprintf(argv_node_tp[1], "%d",node_configuration.so_tp_size);
           sprintf(argv_node_tp[2], "%d",current_node.node_id);
           sprintf(argv_node_tp[3], "%d",current_node.pid+current_node.node_id);
           execve(argv_node_tp[0],argv_node_tp,NULL);
           ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO GENERATE NODE_TP");
           break;
       default:/*father*/
           if (argv_node_tp[1] != NULL) free(argv_node_tp[1]);
           if (argv_node_tp[2] != NULL) free(argv_node_tp[2]);
           if (argv_node_tp[3] != NULL) free(argv_node_tp[3]);
           current_node.type.block.kid_pid = node_tp_pid;
           break;
   }
}

void adv_users_of_block(void){
    int sender_pid = -1;
    int receiver_pid = -1;
  while(queue_is_empty(current_node.transactions_list)==FALSE){
            struct user_msg *u_msg_rep =  (struct user_msg *) malloc(sizeof(struct user_msg));
            DEBUG_ERROR_MESSAGE("NODE TRANSACTION FAILED");
            u_msg_rep->t.t_type = TRANSACTION_SUCCES;
            struct Transaction t = queue_head(current_node.transactions_list);
            sender_pid = t.sender;
            receiver_pid = t.reciver;
            int queue_id_user_proc= get_queueid_by_pid(shm_conf_pointer_node,sender_pid,TRUE);
            if (queue_id_user_proc<0){
                ERROR_MESSAGE("ILLIGAL PID INTO TRANSACTION, NO PIDS FOUND");
                return;
            }
            user_msg_snd(queue_user_id, u_msg_rep, MSG_TRANSACTION_CONFIRMED_TYPE, &t, current_node.pid, TRUE, queue_id_user_proc);
            queue_id_user_proc= get_queueid_by_pid(shm_conf_pointer_node,receiver_pid,TRUE);
            if (queue_id_user_proc<0){
                ERROR_MESSAGE("ILLIGAL PID INTO TRANSACTION, NO PIDS FOUND");
                return;
            }
            user_msg_snd(queue_user_id, u_msg_rep, MSG_TRANSACTION_INCOME_TYPE, &t, current_node.pid, TRUE, queue_id_user_proc);
  }
}
