

#ifndef DGT_SO_PROJECT_21_22_ERRORS_H
#define DGT_SO_PROJECT_21_22_ERRORS_H
#include <string.h>
#include <errno.h>

#define EXIT_PROCEDURE(exit_value)  wait_kids();     \
                                kill_kids();        \
                                free_mem();         \
                                free_sysVar();      \
                                exit(exit_value)
#define ERROR_MESSAGE(err_mex) fprintf(stderr, "%s%s := Error: %s -> | file: %s | line: %d | proc_pid: %d | \n (-) %d\n(-)%s\n %s\n", COLOR_RESET_ANSI_CODE, COLOR_RED_ANSI_CODE,err_mex, __FILE__, __LINE__ ,getpid(), errno, strerror(errno),COLOR_RESET_ANSI_CODE)

#define ERROR_EXIT_SEQUENCE(str) ERROR_MESSAGE(str);\
                            EXIT_PROCEDURE(EXIT_FAILURE)

void wait_kids();

void kill_kids();

void free_mem();

void free_sysVar();
#endif /*DGT_SO_PROJECT_21_22_ERRORS_H*/
