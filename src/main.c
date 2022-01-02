
/*Deprecated: new standard is not implemented for university course constrain*/
#define _GNU_SOURCE
/*If this macro is defined to 1, security hardening is added to various library functions. If def
 * ined to 2, even stricter checks are applied. If defined to 3, the GNU C Library may also use checks that may have
 * an additional performance overhead.*/
#define _FORTIFY_SOURCE 2


/* Std  */
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

/*  LOCAL IMPORT*/

#include "local_lib/headers/glob_vars.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/simulation_errors.h"
#include "local_lib/headers/debug_utility.h"

struct conf simulation_conf;

/*Funzioni di supporto al main*/
void set_signal_handlers(struct sigaction sa);
void signals_handler(int signum);
void create_semaphores(void);
void create_masterbook(void);
/*Variabili di SYS*/
/*TODO: message transaction queue for single user_transaction auto incremento with ## macro*/
int msg_transaction_reports = -1; /*Identifier for message queue*/
int semaphore_start_id = -1;
/*Variabili Globali*/
pid_t main_pid;

int main() {
    /************************************
     *      CONFIGURATION FASE
     * ***********************************/
    if (read_conf(simulation_conf) == TRUE) {
        struct sigaction sa; /*Structure for handling signals*/
        main_pid = getpid();
        set_signal_handlers(sa);
        create_semaphores();
        create_masterbook();

        /*************************************
         *  CREATION OF CHILD PROCESSES FASE *
         * ***********************************/
    }
    return 0;
}

void set_signal_handlers(struct sigaction sa) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("Setting Signals Handlers...");
    memset(&sa, 0, sizeof(sa));/*initialize the structure*/
    sa.sa_handler = signals_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0) {
        ERROR_MESSAGE("Errore Setting Signal Handlers");
        EXIT_PROCEDURE(EXIT_FAILURE);
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("Setting Signals Handlers COMPLETED");
}

/**
 * Handler for the signals SIGINT, SIGTERM, SIGALARM
 * @param signum
 */
void signals_handler(int signum) { /*TODO: Scrivere implementazione*/
    DEBUG_SIGNAL("Signal recived", signum);
    switch (signum) {
        case SIGINT:
        case SIGTERM:
        case SIGALRM:
        default:
            break;
    }
}

void create_semaphores(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF START_SEMAPHORE CHILDREN....");
    semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, IPC_CREAT | IPC_EXCL | 0x600);
    if (semaphore_start_id < 0)ERROR_EXIT_SEQUENCE("IMPOSSIBLE TO CREATE START_SEMAPHORE");
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF START_SEMAPHORE CHILDREN DONE");

    DEBUG_NOTIFY_ACTIVITY_RUNNING("INITIALIZATION OF START_SEMAPHORE CHILDREN....");
    if (semctl(semaphore_start_id, 0, SETVAL, simulation_conf.so_user_num + simulation_conf.so_nodes_num) <
        0)
    ERROR_EXIT_SEQUENCE("IMPOSSIBLE TO INITIALISE SEMAPHORE START CHILDREN");
    DEBUG_NOTIFY_ACTIVITY_DONE("INITIALIZATION OF START_SEMAPHORE CHILDREN DONE");

}

void create_masterbook(){
    /*TODO:da implementare create_masterbook*/
}

