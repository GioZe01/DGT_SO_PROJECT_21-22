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

#ifdef DEBUG

#include "local_lib/headers/debug_utility.h"

#else
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

#define INIT_STATE 0
#define RUNNING_STATE 1

/* Support Function*/

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
 * Make the shm_conf_pointer points to the correct conf shm
 */
void attach_to_shm_conf(void);

/**
 * If it finds node msg of type NODE_TRANSACTION it proccess them
 * and make the aknowledgement
 * @param msg_rep the messagge to be loaded if present in the queue
 * */
void process_node_transaction(struct node_msg *msg_rep);

/**
 * if it finds node msg of type TRANSACTION_TYPE it process them
 * and make the aknowledgement
 * @param msg_rep the messagge to be loaded if present in the queue
 * */
void process_simple_transaction_type(struct node_msg *msg_rep);

/**
 * Connects to the differents queues: master, node's and user's
 * */
void connect_to_queues(void);

/* SysVar */
int state; /* Current state of the node proc*/
int semaphore_start_id = -1; /*Id of the start semaphore arrays for sinc*/

int queue_node_id = -1;/* Identifier of the node queue id */
int queue_user_id = -1; /* Identifier of the user queue id*/
int node_end = 0; /* For value different from 0 the node proc must end*/
int node_id = -1; /* Id of the current node into the snapshots vector*/
struct node current_node; /* Current representation of the node*/
struct conf node_configuration; /* Configuration File representation*/
struct shm_conf *shm_conf_pointer_node; /* Ref to the shm for configuration of the node*/

int main(int argc, char const *argv[]) {
    DEBUG_MESSAGE("NODE PROCESS STARTED");
    struct sigaction sa;
    sigset_t sigmask;
    DEBUG_MESSAGE("NODE STATE SET TO INIT");
    state = INIT_STATE;
    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
    if (check_arguments(argc, argv) && set_signal_handler_node(sa, sigmask)) {
        struct node_msg msg_rep;
        read_conf_node(&node_configuration);
        node_create(&current_node, getpid(), 0, node_configuration.so_tp_size, node_configuration.so_block_size,
                    node_configuration.so_reward, &calc_reward);

        /*-----------------------*/
        /*  CONNECTING TO QUEUES *
        /*-----------------------*/
        connect_to_queues();
        /*-------------------------*/
        /*  SHARED MEM  CONFIG     *
        /*-------------------------*/
        attach_to_shm_conf();
        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/

        /*---------------------------*/
        /*  SEMAPHORES CREATION      *
        /*---------------------------*/
        semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
        if (semaphore_start_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN ID OF START SEM"); }
        if (semaphore_lock(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
        }
        DEBUG_MESSAGE("NODE READY, ON START_SEM");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO WAIT FOR START");
        }
        state = RUNNING_STATE;

        /****************************************
         *      PROCESSING OF TRANSACTION FASE  *
         * **************************************/
        while (node_end != 1) {
            /*TODO: verificare*/
            process_node_transaction(&msg_rep);
            process_simple_transaction_type(&msg_rep);
#ifdef DEBUG
            node_msg_print(&msg_rep);
#endif
        }

    }
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

/**
 * Calculate the balance of the current user_proc
 * @param budget the current budget that is available for the user_proc;
 * @return the value of the balance
 */
Bool check_arguments(int argc, char const *argv[]) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
    if (argc < 2) {
        ERROR_EXIT_SEQUENCE_NODE("MISSING ARGUMENT");
    }
    node_id = atoi(argv[1]);
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}

Bool set_signal_handler_node(struct sigaction sa, sigset_t sigmask) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("SETTING SIGNAL MASK...");
    sigemptyset(&sigmask);/*Creating an empty mask*/
    sigaddset(&sigmask, SIGALRM);/*Adding signal to the mask*/
    DEBUG_NOTIFY_ACTIVITY_DONE("SETTING SIGNAL MASK DONE");

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signals_handler;
    sa.sa_mask = sigmask;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0
            ) {
        ERROR_EXIT_SEQUENCE_NODE("ERROR DURING THE CREATION OF THE SIG HANDLER ");
    }
    return TRUE;
}

/**
 * Set the signal handler and signal mask for the user_proc
 * @return TRUE if success, FALSE otherwise.
 */
void signals_handler(int signum) {
    DEBUG_SIGNAL("SIGNAL RECEIVED ", signum);
    switch (signum) {
        case SIGINT:
            alarm(0);/*pending alarm removed*/
            EXIT_PROCEDURE_NODE(0);
        case SIGALRM:
            break;

    }
}

void free_sysVar_node() {
    int semaphore_start_value;
    if (state == INIT_STATE && semaphore_start_id >= 0) {
        semaphore_start_value = semctl(semaphore_start_id, 0, GETVAL);
        if (semaphore_start_value < 0) ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0) {
            if (semaphore_lock(semaphore_start_id, 0) < 0)
                ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }

    if (queue_node_id >= 0 && msgctl(queue_node_id, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO DELETE MESSAGE QUEUE OF NODE");
    }
}

void free_mem_node() {
    free_node(&current_node);
}

void attach_to_shm_conf(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("ATTACHING TO SHM...");
    int shm_conf_id = -1;/* id to the shm_conf*/
    shm_conf_id = shmget(SHM_CONFIGURATION, sizeof(struct shm_conf), 0600);
    if (shm_conf_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO ACCESS SHM CONF"); }
    shm_conf_pointer_node = shmat(shm_conf_id, NULL, 0);
    if (shm_conf_pointer_node == (void *) -1) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO SHM CONF"); }
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO SHM DONE");
}

void process_node_transaction(struct node_msg *msg_rep) {
    if (node_msg_receive(queue_node_id, msg_rep, node_id) == 0) {
        /*Checking for transaction coming from node*/
        DEBUG_MESSAGE("NODE TRANSACTION RECEIVED");

    }
}

void process_simple_transaction_type(struct node_msg *msg_rep) {
    struct user_msg *u_msg_rep;
    if (node_msg_receive(queue_node_id, msg_rep, node_id - MSG_NODE_ORIGIN_TYPE) == 0) {
        /*Checking for transaction type*/
        DEBUG_MESSAGE("NODE TRANSACTION TYPE RECEIVED");
        if (get_num_transactions(current_node.transaction_pool) < node_configuration.so_tp_size) {
            queue_append(current_node.transaction_pool, msg_rep->t);
        } else {
            DEBUG_ERROR_MESSAGE("NODE TRANSACTION FAILED");
            u_msg_rep->t.t_type = TRANSACTION_FAILED;
            user_msg_snd(queue_user_id, u_msg_rep, MSG_TRANSACTION_FAILED_TYPE, &msg_rep->t, current_node.pid, TRUE);
        }
    }
}

void connect_to_queues(void) {
    /*TODO: Aggiungerla come optional alla compilazione*/
    queue_node_id = msgget(NODES_QUEUE_KEY, 0600);
    if (queue_node_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO NODE MESSAGE QUEUE"); }
    queue_user_id = msgget(USERS_QUEUE_KEY, 0600);
    if (queue_user_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO USER QUEUE"); }
}
