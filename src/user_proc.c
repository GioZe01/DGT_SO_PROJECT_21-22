#define _GNU_SOURCE
/*  Standard Library  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
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

Bool set_signal_handler(struct sigaction sa, sigset_t sigmask);

Bool read_conf(struct conf *simulation_conf);

void configure_shm();

/*  SysV  */

int state;
int semaphore_start_id = -1;
int queue_report_id = -1; /* -1 is the value if it is not initialized */
struct user_transaction current_user;

int main(int arc, char const *argv[]) {
    DEBUG_MESSAGE("USER PROCESS STARTED");
    struct sigaction sa;
    struct user_msg msg_rep;
    sigset_t sigmask; /* sinal mask */

    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/

    DEBUG_MESSAGE("USER STATE IS SET TO INIT");
    state = INIT_STATE;

    if (check_argument(arc, argv) && set_signal_handler(sa, sigmask)) {
        /*  VARIABLE INITIALIZATION */
        int semaphore_start_value = -1;
        char * buffer[sizeof (int)*8+1];
        struct conf configuration;
        read_conf(&configuration);
        user_create(&current_user, configuration.so_buget_init, getpid(), calc_balance);

        /*-------------------------*/
        /*  CREAZIONE QUEUE REPORT *
        /*-------------------------*/
        /*TODO: Aggiungerla come optional alla compilazione*/
        int_to_hex(current_user.pid, buffer);
        queue_report_id = msgget(buffer,IPC_CREAT | IPC_EXCL | 0600);
        printf("----------------USER_QUEUE ID: %d\n",queue_report_id);
        if (queue_report_id < 0) { ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CREATE THE MESSAGE QUEUE"); }

        /*-------------------------*/
        /*  SHARED MEM  CONFIG     *
        /*-------------------------*/
        /*configure_shm(); TODO: convert*/
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
        if (semaphore_lock(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
        }
        DEBUG_MESSAGE("USER READY, WAITING FOR SEMAPHORE TO FREE");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("ERROR DURING WAITING START_SEMAPHORE UNLOCK");
        }
        state = RUNNING_STATE;

        /*-------------------------------------------*
         *  GETTING THE KNOWLEDGE OF USERS id_to_pid *
         * ------------------------------------------*/

        if (msgrcv(queue_report_id, &msg_rep, sizeof(msg_rep) - sizeof(msg_rep.type),
                   MSG_CONFIG_TYPE, 0) < 0 &&
            errno == EINTR) {
            ERROR_EXIT_SEQUENCE_USER("MISSED CONFIG ON MESSAGE QUEUE");
        }
        printf("\nCONFIGURAZIONE RICEVUTA: %d\n", msg_rep.data.users_id_to_pid[0]);
        /****************************************
         *      GENERATION OF TRANSACTION FASE *
         * **************************************/
        raise(SIGALRM);
        while (1) pause(); /*Waiting for a signal to come*/


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

    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}

/**
 * Set the signal handler and signal mask for the user_proc
 * @return TRUE if success, FALSE otherwise.
 */
Bool set_signal_handler(struct sigaction sa, sigset_t sigmask) {

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
        ERROR_EXIT_SEQUENCE_USER("ERRORE DURING THE CREATION OF THE SIG HANDLER ");
    }
    return TRUE;
}

/* TODO: Convert or remove
void configure_shm() {
    int shm_user_id = -1;
    DEBUG_NOTIFY_ACTIVITY_RUNNING("RETRIEVING SHM ID...");
    shm_user_id = shmget(SHM_USERS_PROC_KEY, sizeof(struct user_snapshot), 0);
    if (shm_user_id < 0) {
        ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO GET THE USER PROC SHARED MEMORY ");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("RETRIEVING SHM ID DONE");

    DEBUG_NOTIFY_ACTIVITY_RUNNING("ATTACHING TO THE SHM...");
    users = (struct user_snapshot *) shmat(shm_user_id, NULL, 0);
    if (users == (void *) -1) {
        ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CONNECT TO THE SHARED MEMORY ");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO THE SHM DONE");
}
*/
void signals_handler(int signum) {
    DEBUG_SIGNAL("SIGNAL RECEIVED", signum);
    switch (signum) {
        case SIGINT:
            alarm(0);/* pending alarm canceld*/
            EXIT_PROCEDURE_USER(0);
        case SIGALRM: /*    Generate a new transaction  */
            DEBUG_NOTIFY_ACTIVITY_RUNNING("GENERATING A NEW TRANSACTION...");
            /*generate_transaction(&current_user);*/
            DEBUG_NOTIFY_ACTIVITY_DONE("GENERATING A NEW TRANSACTION DONE");
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
