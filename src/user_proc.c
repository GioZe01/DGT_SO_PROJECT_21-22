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
#include "local_lib/headers/debug_utility.h"
#include "local_lib/headers/transaction_list.h"
#include "local_lib/headers/user_transaction.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/conf_file.h"
/*  Constant definition */
#define INIT_STATE 0
#define RUNNING_STATE 1

/*  Function def. */
void handler(int signum);

void free_sysVariable(void);

/*  Helper  */
Bool check_argument(int arc, char const *argv[]);

int calc_balance(unsigned int budget);

Bool set_signal_handler(struct sigaction sa, sigset_t sigmask);

/*  SysV  */
int budget;
int state;
int semaphore_start_id = -1;
int queue_report_id = -1; /* -1 is the value if it is not initialized */
struct user_transaction curre_user;

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
        curre_user = user_create(budget, getpid(), calc_balance);

        /*-------------------------*/
        /*  CREAZINE DEI SEMAFORI  *
        /*-------------------------*/

        /*TODO: need a semafore for reading into the message queue*/
        semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
        if (semaphore_start_id < 0) ERROR_EXIT_SEQUENCE("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");

        /*-------------------------*/
        /*  CREAZIONE QUEUE REPORT *
        /*-------------------------*/
        queue_report_id = msgget(curre_user.pid, IPC_CREAT | IPC_EXCL | 0600);
        if (queue_report_id < 0) ERROR_EXIT_SEQUENCE("IMPOSSIBLE TO CREATE THE MESSAGE QUEUE");


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
        DEBUG_NOTIFY_ACTIVITY_RUNNING("SEMAPHORE START UNLOAKING....")
        semaphore_start_Value = semctl(semaphore_start_id, 0, GETVAL);/* 0 as reading op. */
        if (semaphore_start_Value < 0) ERROR_EXIT_SEQUENCE("IMPOSSIBLE TO RETRIVE INFORMATION FROM START_SEMAPHORE");
        if (semaphore_start_Value != 0 && semaphore_lock(semaphore_start_id, 0) < 0)
        ERROR_EXIT_SEQUENCE("ERROR OCCURED DURING UNLOCK OF THE START_SEMAPHORE");

        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0)
        ERROR_EXIT_SEQUENCE("ERROR DURING WAITING START_SEMAPHORE UNLOAK");

        state = RUNNING_STATE;
        DEBUG_NOTIFY_ACTIVITY_DONE("SEMAPHORE START UNLOAKING DONE")
        /*------------------------------*/
        /*  CONNESSIONE AI QUEUE REPORT *
        /*------------------------------*/

    }

    ERROR_EXIT_SEQUENCE("CREATION OF USER_PROC FAILED DUE TO: Arg or Signal handler creation failure");
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
    budget = argv[1];
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
    sa.sa_handler = handler;
    sa.sa_mask = sigmask;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0 ||
        sigaction(SIGUSR1, &sa, NULL) < 0) {
        ERROR_EXIT_SEQUENCE("ERRORE DURING THE CREATION OF THE SIG HANDLER ");
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