

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
#define DEBUG_MESSAGE(mex) fprintf(stderr, "%s%s [DEBUG]:= | file_in: %s | pid: %d | line %d |\n -> %s\n%s")
#define DEBUG_SIGNAL(mex) fprintf(stderr, "%s%s [DEBUG]:= | file_in: %s | pid: %d | line %d |\n -> %s\n%s")
#define DEBUG_ERROR_MESSAGE(mex) fprintf(stderr, "%s%s [DEBUG]:= | file_in: %s | pid: %d | line %d |\n -> %s\n%s")
#endif

struct conf simulation_conf;

/*Funzioni di supporto al main*/
Bool read_conf();

/*Variabili di SYS*/

/*Variabili Globali*/
pid_t main_pid;

int main() {
    read_conf();
    main_pid = getpid();
    return 0;
}

Bool read_conf() {
    switch (load_configuration(&simulation_conf)) {
        case 0:
            break;
        case -1:
            MESSAGE_ERROR_FILE_CONF("Missing file or Empty");
            EXIT_PROCEDURE(EXIT_FAILURE);
        case -2:
            MESSAGE_ERROR_FILE_CONF("Broken simulation logic, check conf value");
            EXIT_PROCEDURE(EXIT_FAILURE);
        case -3:
            MESSAGE_ERROR_FILE_CONF("Not enough users for nodes");
            EXIT_PROCEDURE(EXIT_FAILURE);
        case -4:
            MESSAGE_ERROR_FILE_CONF("Min Max Excecution time wrong");
            EXIT_PROCEDURE(EXIT_FAILURE);
        case -5:
            MESSAGE_ERROR_FILE_CONF("Node reward is over possibilities of users");
            EXIT_PROCEDURE(EXIT_FAILURE);
    }
}

