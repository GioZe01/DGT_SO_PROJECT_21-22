#define _GNU_SOURCE
#define _FORTIFY_SOURCE 2
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
#include "local_lib/headers/glob_vars.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/simulation_errors.h"
#include "local_lib/headers/semaphore.h"
#ifdef DEBUG
#include "local_lib/headers/debug_utility"
#else
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

/* Function def. */

void signals_handler(int signum);

Bool read_conf(struct conf simulation_conf);
/* Helper */
Bool set_signal_handler(struct sigaction sa, sigset_t sigmask);

Bool check_arguments(int argc, char const *argv);

/* SysVar */
int semaphore_start_id = -1;
/*struct node node;*/
int main(int argc, char const *argv[]) {
    DEBUG_MESSAGE("NODE PROCESS STARTED");
    struct sigaction sa;
    sigset_t sigmask;


    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
    if (check_arguments(argc, argv) && set_signal_handler(sa, sigmask)) {
       int semaphore_start_value = -1;
       struct conf configuration;
       read_conf(configuration);
       /*node_create(node)*/

        /*---------------------------*/
        /*  SEMAPHORES CREATOIN FASE *
        /*---------------------------*/
        semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
        if (semaphore_lock(semaphore_start_id,0)< 0) { ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE"); }
        DEBUG_MESSAGE("READY, ON START_SEM");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBILE TO WAIT FOR START");
        }

        /*-------------------------*/
        /*  CREAZIONE QUEUE REPORT *
        /*-------------------------*/
    }
    return 0;
}

/**
 * Load and read the configuration, in case of error during loading close the proc. with EXIT_FAILURE
 * @return TRUE if ALL OK
 */
Bool read_conf(struct conf simulation_conf) {
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

/**
 * Set the signal handler and signal mask for the user_proc
 * @return TRUE if success, FALSE otherwise.
 */
void signals_handler(int signum) {

}
void free_sysVar_node(){

}
void free_meme_node(){

}
