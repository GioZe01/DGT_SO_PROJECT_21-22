/*  Standard Library  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
/*  Sys Library */
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
/*  Local Library */
#include "local_lib/headers/simulation_errors.h"
#include "local_lib/headers/transaction_list.h"
#include "local_lib/headers/user_transaction.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/user_msg_report.h"
#include "local_lib/headers/node_msg_report.h"
#include "local_lib/headers/master_msg_report.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/conf_shm.h"

#define EXIT_PROCEDURE_USER(exit_value) free_mem_user();         \
                                free_sysVar_user();      \
                                exit(exit_value)
#ifdef DEBUG

#include "local_lib/headers/debug_utility.h"

#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

/*  Constant definition */
#define INIT_STATE 0
#define RUNNING_STATE 1

/*  Function def. */
void signals_handler(int signum);

/*  Helper  */
Bool check_argument(int arc, char const *argv[]);

Bool set_signal_handler_user(struct sigaction sa, sigset_t sigmask);

Bool read_conf(struct conf *simulation_conf);

int send_to_node(void);

void attach_to_shm_conf(void);

/*  SysV  */

int state; /* Current state of the user proc*/
int semaphore_start_id = -1; /**/
int queue_report_id = -1; /* Identifier of the user queue id*/
int users_snapshot[][2];/* Contains the ref to the pid_t of the users and the queue id*/
int nodes_snapshot[][2];/* Contains the ref to the pid_t of the nodes and the queue id*/
int user_id = -1; /*Id of the current user into the snapshots vectors*/
struct user_transaction current_user; /* Current representation of the user*/
struct conf configuration; /* Configuration File representation */
struct shm_conf *shm_conf_pointer; /* Ref to the shm fir configuration of the user*/

int main(int arc, char const *argv[]) {
    DEBUG_MESSAGE("USER PROCESS STARTED");
    struct sigaction sa;
    struct user_msg msg_rep;
    struct timespec gen_sleep;
    int start_sem_value;
    sigset_t sigmask; /* sinal mask */

    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/

    DEBUG_MESSAGE("USER STATE IS SET TO INIT");
    state = INIT_STATE;

    if (check_argument(arc, argv) && set_signal_handler_user(sa, sigmask)) {
        /*  VARIABLE INITIALIZATION */
        read_conf(&configuration);
        user_create(&current_user, configuration.so_buget_init, getpid(), &calc_balance, update_cash_flow);
        gen_sleep.tv_sec = 0;
        /*--------------------------------------*/
        /*  CONNECTING TO THE USER REPORT QUEUE *
        /*--------------------------------------*/
        /*TODO: Aggiungerla come optional alla compilazione*/
        queue_report_id = msgget(USERS_QUEUE_KEY, 0600);
        if (queue_report_id < 0) { ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CREATE THE MESSAGE QUEUE"); }

        /*-------------------------*/
        /*  SHARED MEM  CONFIG     *
        /*-------------------------*/
        attach_to_shm_conf();

        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/

        /*------------------------------------*/
        /*  WAITING START SEMAPHORE TO UNLOCK *
         * -----------------------------------*
         * INFO:                              *
         * unlock is done if sem hasn't 0 as  *
         * value                              *
        /*------------------------------------*/
        /*TODO: need a semafore for reading into the message queue*/
        semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
        if (semaphore_start_id < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN ID OF START SEM.");
        }
        start_sem_value = semctl(semaphore_start_id, 0, GETVAL);
        if (start_sem_value < 0) { ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN INFO FROM START SEM."); }
        printf("------------SEMAPHORE START VALUE: %d\n", start_sem_value);
        if (start_sem_value != 0 && semaphore_lock(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
        }
        DEBUG_MESSAGE("USER READY, WAITING FOR SEMAPHORE TO FREE");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("ERROR DURING WAITING START_SEMAPHORE UNLOCK");
        }
        state = RUNNING_STATE;

        /****************************************
         *      GENERATION OF TRANSACTION FASE *
         * **************************************/
        while (current_user.u_balance > 2) {
            DEBUG_MESSAGE("TRANSACTION ALLOWED");
            if (generate_transaction(&current_user, current_user.pid, users_snapshot) < 0) {
                ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO GENERATE TRANSACTION");
            }
#ifdef DEBUG
            queue_print(current_user.in_process);
#endif
            gen_sleep.tv_nsec =
                    (rand() % (configuration.so_max_trans_gen_nsec - configuration.so_min_trans_gen_nsec + 1)) +
                    configuration.so_min_trans_gen_nsec;
#ifdef U_CASHING
            /*TODO: make cashing*/
#endif

            /*SENDING TRANSACTION TO THE NODE*/

            if (send_to_node() < 0) {
                ERROR_MESSAGE("IMPOSSIBLE TO SEND TO THE NODE");
            }
            /*
             * TODO: check for the retry to send : cand do with while then abort and notify master
             * */
            nanosleep(&gen_sleep, (void *) NULL);
        }

        DEBUG_MESSAGE("USER ENDED -----------------------------");
        EXIT_PROCEDURE_USER(0);
    }

    ERROR_EXIT_SEQUENCE_USER("CREATION OF USER_PROC FAILED DUE TO: Arg or Signal handler creation failure");
}

/**
 * Check if the argument respect initialization value for the user_proc
 * @param arc number of value in argv
 * @param argv argument pass in the main function
 * @return  TRUE if all OK, otherwise FALSE
 */
Bool check_argument(int arc, char const *argv[]) {
    /*TODO: controllo dell arc*/
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
    if (arc < 2) {
        ERROR_EXIT_SEQUENCE_USER("MISSING ARGUMENT");
    }
    user_id = atoi(argv[1]);
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}

/**
 * Set the signal handler and signal mask for the user_proc
 * @return TRUE if success, FALSE otherwise.
 */
Bool set_signal_handler_user(struct sigaction sa, sigset_t sigmask) {

    DEBUG_NOTIFY_ACTIVITY_RUNNING("SETTING SIGNAL MASK...");
    sigemptyset(&sigmask);/*Creating an empty mask*/
    sigaddset(&sigmask, SIGALRM);/*Adding signal to the mask*/
    DEBUG_NOTIFY_ACTIVITY_DONE("SETTING SIGNAL MASK DONE");

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signals_handler;
    sa.sa_mask = sigmask;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0 ||
        sigaction(SIGUSR1, &sa, NULL) < 0) {
        ERROR_EXIT_SEQUENCE_USER("ERROR DURING THE CREATION OF THE SIG HANDLER ");
    }
    return TRUE;
}

void signals_handler(int signum) {
    DEBUG_SIGNAL("SIGNAL RECEIVED", signum);
    switch (signum) {
        case SIGINT:
            alarm(0);/* pending alarm canceld*/
            EXIT_PROCEDURE_USER(0);
        case SIGALRM: /*    Generate a new transaction  */
            DEBUG_NOTIFY_ACTIVITY_RUNNING("GENERATING A NEW TRANSACTION FROM SIG...");
            if (generate_transaction(&current_user, current_user.pid, users_snapshot) < 0) {
                ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO GENERATE TRANSACTION");
            }
            DEBUG_NOTIFY_ACTIVITY_DONE("GENERATING A NEW TRANSACTION FROM SIG DONE");
        default:
            break;
    }
}


void free_mem_user() {
    free_user(&current_user);
}

void free_sysVar_user() {
    /*TODO: aggiungi altri */
    int semaphore_start_value;
    if (state == INIT_STATE && semaphore_start_id >= 0) {
        semaphore_start_value = semctl(semaphore_start_id, 0, GETVAL);
        if (semaphore_start_value < 0) ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0) {
            if (semaphore_lock(semaphore_start_id, 0) < 0)
                ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }

    if (queue_report_id >= 0 && msgctl(queue_report_id, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO DELETE MESSAGE QUEUE OF USER");
    }
}

/**
 * Load and read the configuration, in case of error during loading close the proc. with EXIT_FAILURE
 * @return TRUE if ALL OK
 */
Bool read_conf(struct conf *simulation_conf) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION...");
    switch (load_configuration(simulation_conf)) {
        case 0:
            break;
        case -1:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: MISSING FILE OR EMPTY");
        case -2:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: BROKEN SIMULTATION LOGIC, CHECK CONF. VALUE");
        case -3:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: NOT ENOUGH USERS FOR NODES");
        case -4:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: MIN MAX EXECUTION TIME WRONG");
        case -5:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: NODE REWARD IS OVER POSSIBILITIES OF USERS");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CONFIGURATION LOADED");
    return TRUE;
}

int send_to_node(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("SENDING TRANSACTION TO THE NODE...");
    int node_num = (rand() % (nodes_snapshot[0][0])) + 1;
    struct node_msg msg;
    struct Transaction t = queue_last(current_user.in_process);
    if (node_msg_snd(NODES_QUEUE_KEY, &msg, nodes_snapshot[node_num][2], &t,
                     current_user.pid, TRUE) < 0) { return -1; }
#ifdef DEBUG
    node_msg_print(&msg);
#endif
    return 0;

}

void attach_to_shm_conf(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("ATTACHING TO SHM...");
    int shm_conf_id = -1;/* id to the shm_conf*/
    shm_conf_id = shmget(SHM_CONFIGURATION, sizeof(struct shm_conf), 0600);
    if (shm_conf_id < 0) { ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO ACCESS SHM CONF"); }
    shm_conf_pointer = shmat(shm_conf_id, NULL, 0);
    if (shm_conf_pointer == (void *) -1) { ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CONNECT TO SHM CONF"); }
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO SHM DONE");
}