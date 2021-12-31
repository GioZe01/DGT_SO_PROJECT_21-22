

#define _POSIX_C_SOURCE 200809L
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

/*  DEBUG_MESSAGE definitions:  */
#ifdef DEBUG
#define DEBUG_NOTIFY_ACTIVITY(mex) print("%s%s [DEBUG-NOTIFY]:= %s\n", COLOR_RESET_ANSI_CODE,COLOR_YELLOW_ANSI_CODE,mex)
#define DEBUG_MESSAGE(mex) fprintf(stderr, "%s%s [DEBUG]:= | file_in: %s | pid: %d | line %d |\n -> %s\n%s")
#define DEBUG_SIGNAL(mex, signum) fprintf(stderr, "%s%s [DEBUG]:= | file_in: %s | pid: %d | line %d |\n -> %s\n%s")
#define DEBUG_ERROR_MESSAGE(mex) fprintf(stderr, "%s%s [DEBUG-SIGNAL]:= | file_in: %s | pid: %d | line %d |\n -> %s, {%d}\n%s",COLOR_RESET_ANSI_CODE, COLOR_YELLOW_ANSI_CODE, __FILE__, getpid(), __LINE__,mex,signum,COLOR_RESET_ANSI_CODE)
#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

struct conf simulation_conf;

/*Funzioni di supporto al main*/
Bool read_conf();

void signals_handler(int signum);
/*Variabili di SYS*/
/*TODO: message transaction queue for single user auto incremento with ## macro*/
int msg_transaction_reports = -1; /*Identifier for message queue*/
/*Variabili Globali*/
pid_t main_pid;

int main() {
    if (read_conf() == TRUE) {
        struct sigaction sa; /*Structure for handling signals*/
        main_pid = getpid();
        DEBUG_NOTIFY_ACTIVITY("Setting Signals Handlers...");

        memset(&sa, 0, sizeof(sa));/*initialize the structure*/
        sa.sa_handler = signals_handler;
        if (sigaction(SIGINT, &sa, NULL) < 0 ||
            sigaction(SIGTERM, &sa, NULL) < 0 ||
            sigaction(SIGALRM, &sa, NULL) < 0) {
            ERROR_MESSAGE("Errore Setting Signal Handlers");
            EXIT_PROCEDURE(EXIT_FAILURE);
        }
        DEBUG_NOTIFY_ACTIVITY("Setting Signals Handlers COMPLETED");

    }
    return 0;
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

/**
 * Load and read the configuration, in case of error during loading close the proc. with EXIT_FAILURE
 * @return TRUE if ALL OK
 */
Bool read_conf() {

    DEBUG_NOTIFY_ACTIVITY("LOADING CONFIGURATION...");
    switch (load_configuration(&simulation_conf)) {
        case 0:
            break;
        case -1:
        ERROR_EXIT_SEQUENCE(" during conf. loading: Missing File or Empty");
        case -2:
        ERROR_EXIT_SEQUENCE(" during conf. loading: Broken simultation logic, check conf. value");
        case -3:
        ERROR_EXIT_SEQUENCE(" during conf. loading: Not enough users for nodes");
        case -4:
        ERROR_EXIT_SEQUENCE(" during conf. loading: Min Max Execution time wrong");
        case -5:
        ERROR_EXIT_SEQUENCE(" during conf. loading: Node reward is over possibilities of users");
    }
    DEBUG_NOTIFY_ACTIVITY("CONFIGURATION LOADED");
    return TRUE;
}

