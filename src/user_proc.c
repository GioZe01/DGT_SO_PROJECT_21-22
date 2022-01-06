#define _GNU_SOURCE
/*  Standard Library  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
/*  Sys Library */
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
/*  Local Library */
#include "local_lib/headers/simulation_errors.h"

#ifdef DEBUG
#include "local_lib/headers/debug_utility.h"
#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

#include "local_lib/headers/transaction_list.h"
#include "local_lib/headers/user_transaction.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/conf_file.h"
/*  Constant definition */
#define INIT_STATE 0
#define RUNNING_STATE 1

/*  Function def. */
void signals_handler(int signum);

/*  Helper  */
Bool check_argument(int arc, char const *argv[]);

int calc_balance(unsigned int budget);

Bool set_signal_handler(struct sigaction sa, sigset_t sigmask);

Bool read_conf(struct conf simulation_conf);
/*  SysV  */
int budget;
int state;
int semaphore_start_id = -1;
int queue_report_id = -1; /* -1 is the value if it is not initialized */
struct user_transaction current_user;

int main(int arc, char const *argv[]) {
    DEBUG_MESSAGE("USER PROCESS STARTED")
    struct sigaction sa;
    sigset_t sigmask; /* sinal mask */

    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/

    DEBUG_MESSAGE("USER STATE IS SET TO INIT");
    state = INIT_STATE;

    if (check_argument(arc, argv) && set_signal_handler(sa, sigmask)) {
        /*  VARIABLE INITIALIZATION */
        int semaphore_start_Value = -1;
        struct conf configuration;

        read_conf(configuration);
        budget = configuration.so_buget_init;
        current_user = user_create(budget, getpid(), calc_balance);

        /*-------------------------*/
        /*  CREAZINE DEI SEMAFORI  *
        /*-------------------------*/

        /*TODO: need a semafore for reading into the message queue*/
        semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
        if (semaphore_start_id < 0) { ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE"); }

        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBILE TO WAIT FOR START");
        }
        /*-------------------------*/
        /*  CREAZIONE QUEUE REPORT *
        /*-------------------------*/
        queue_report_id = msgget(current_user.pid, IPC_CREAT | IPC_EXCL | 0600);
        if (queue_report_id < 0) {ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CREATE THE MESSAGE QUEUE");}


        DEBUG_MESSAGE("USER READY, WAINT FOR SEMAPHORE TO FREE");


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
        DEBUG_NOTIFY_ACTIVITY_RUNNING("SEMAPHORE START UNLOAKING....");
        semaphore_start_Value = semctl(semaphore_start_id, 0, GETVAL);/* 0 as reading op. */
        if (semaphore_start_Value < 0){
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO RETRIVE INFORMATION FROM START_SEMAPHORE");}
        if (semaphore_start_Value != 0 && semaphore_lock(semaphore_start_id, 0) < 0){
            ERROR_EXIT_SEQUENCE_USER("ERROR OCCURED DURING UNLOCK OF THE START_SEMAPHORE");}

        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0){
            ERROR_EXIT_SEQUENCE_USER("ERROR DURING WAITING START_SEMAPHORE UNLOAK");
        }
        state = RUNNING_STATE;
        DEBUG_NOTIFY_ACTIVITY_DONE("SEMAPHORE START UNLOAKING DONE");
        /*------------------------------*/
        /*  CONNESSIONE AI QUEUE REPORT *
        /*------------------------------*/
        EXIT_PROCEDURE_USER(0);

    }

    ERROR_EXIT_SEQUENCE_USER("CREATION OF USER_PROC FAILED DUE TO: Arg or Signal handler creation failure");
}

/**
 * Check if the argument respect initialization value for the user_proc
 * @param arc numer of value in argv
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

/**
 * Calculate the balance of the current user_proc
 * @param budget the current budget that is available for the user_proc;
 * @return the value of the balance
 */
int calc_balance(unsigned int budget) {
    /*TODO: implement calc_balance*/
}

struct user_snapshot *get_user_snapshot(struct user_transaction user) {
    /*TODO: implement get_user_snapshot*/
}

void signals_handler(int signum) {
    DEBUG_SIGNAL("SIGNAL RECEIVED", signum);
    switch (signum) {
        case SIGINT:
            alarm(0);
            EXIT_PROCEDURE_USER(0);
    }
}


void free_mem_user() {
    free_user(current_user);
}

void free_sysVar_user() {
    /*TODO: aggiungi altri */
    int semaphore_start_value;
    if (state == INIT_STATE && semaphore_start_id >= 0) {
        semaphore_start_value = semctl(semaphore_start_id, 0, GETVAL);
        if (semaphore_start_value < 0) ERROR_MESSAGE("IMPOSSIBLE TO RETRIVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0) {
            if (semaphore_lock(semaphore_start_id, 0) < 0)
                ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }
}

/**
 * Load and read the configuration, in case of error during loading close the proc. with EXIT_FAILURE
 * @return TRUE if ALL OK
 */
Bool read_conf(struct conf simulation_conf) {
    printf("ENTRATO");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION...");
    switch (load_configuration(&simulation_conf)) {
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
