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
#include "local_lib/headers/node_msg_report.h"

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

/* Function def. */

void signals_handler(int signum);

Bool read_conf_node(struct conf *simulation_conf);

Bool set_signal_handler_node(struct sigaction sa, sigset_t sigmask);

Bool check_arguments(int argc, char const *argv);

int aknowledge_data(struct node_msg msg);

/* SysVar */
int state;
int semaphore_start_id = -1;
int queue_node_id = -1;
int *users_queue_ids;
int *nodes_pids;
int *nodes_queue_ids;
struct node current_node;
struct conf node_configuration;

/*struct node node;*/
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
                    node_configuration.so_reward, calc_reward);

        /*-----------------------------------*/
        /*  CONNECTING TO NODE REPORT QUEUE  *
        /*-----------------------------------*/
        /*TODO: Aggiungerla come optional alla compilazione*/
        queue_node_id = msgget(NODES_QUEUE_KEY, 0600);
        printf("----------------USER_QUEUE ID: %d\n", queue_node_id);
        if (queue_node_id < 0) { ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CREATE THE MESSAGE QUEUE"); }

        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/

        /*---------------------------*/
        /*  SEMAPHORES CREATION      *
        /*---------------------------*/
        semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
        if (semaphore_lock(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
        }
        DEBUG_MESSAGE("NODE READY, ON START_SEM");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO WAIT FOR START");
        }
        state = RUNNING_STATE;
        /*-------------------------------------------*
         *  GETTING THE KNOWLEDGE OF USERS id_to_pid *
         * ------------------------------------------*/

        if (queue_node_id == -1 && msgrcv(queue_node_id, &msg_rep, sizeof(msg_rep) - sizeof(msg_rep.type),
                                          current_node.pid - MSG_NODE_ORIGIN_TYPE, 0) < 0 &&
            errno == EINTR) {
            ERROR_EXIT_SEQUENCE_NODE("MISSED CONFIG ON MESSAGE QUEUE");
        }
#ifdef DEBUG
        printf("\nCONFIGURATION RECEIVED: %d\n", msg_rep.data.conf_data.nodes_pids[0]);
#endif
        aknowledge_data(msg_rep);
    }
    EXIT_PROCEDURE_NODE(0);
}

/**
 * Load and read the configuration, in case of error during loading close the proc. with EXIT_FAILURE
 * @return TRUE if ALL OK
 */
Bool read_conf_node(struct conf *simulation_conf) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION...");
    switch (load_configuration(&simulation_conf)) {
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
Bool check_arguments(int argc, char const *argv) {
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
            break;
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
    free(nodes_pids);
    free(nodes_queue_ids);
    free(users_queue_ids);
}

int aknowledge_data(struct node_msg msg) {
    switch (msg.type) {
        case MSG_CONFIG_TYPE:
            users_queue_ids = msg.data.conf_data.users_queues_ids;
            nodes_pids = msg.data.conf_data.nodes_pids;
            nodes_queue_ids = msg.data.conf_data.nodes_queues_ids;
            if (nodes_queue_ids[0] != nodes_pids[0]) {
                ERROR_MESSAGE("NODES QUEUES IDS HAS LENGTH DIFFERENT THAT NODES PIDS");
                return -1;
            }
            if (users_queue_ids[0] == 0) {
                ERROR_MESSAGE("NO USERS TO ADVICE TRANSACTIONS DONE");
                return -1;
            }
            break;
    }
    return 0;
}
