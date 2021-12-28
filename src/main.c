//
// Created by Gio on 12/28/2021.
//

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

#define EXIT_PROCEDURE(exit_value)  wait_kids()     \
                                kill_kids();        \
                                free_mem();         \
                                free_sysVar();      \
                                exit(exit_value);   \

#define ERROR_MESSAGE(err_mex) fprintf(stderr, "%s%s := Errore -> | file_out: %s | line: %s | proc_pid: %d | \n (-) %s\n (-){%d} %s\n%s\n", ANSI_COLOR_RESET, ANSI_COLOR_RED, __FILE__, __LINE_ ,getpid(), errno, strerror(errno), ANSI_COLOR_RESET)
/*  DEBUG_MESSAGE definitions:  */
#ifdef DEBUG
#define DEBUG_MESSAGE(mex) fprintf()
#define DEBUG_SIGNAL(mex) fprintf()
#define DEBUG_ERROR_MESSAGE(mex) fprintf()
#endif


struct conf simulation_conf;