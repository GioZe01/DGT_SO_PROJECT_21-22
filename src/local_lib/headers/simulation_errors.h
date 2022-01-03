#ifndef DGT_SO_PROJECT_21_22_SIMULATION_ERRORS_H
#define DGT_SO_PROJECT_21_22_SIMULATION_ERRORS_H

#include <string.h>
#include <errno.h>
#include "glob_vars.h"

#define EXIT_PROCEDURE_MAIN(exit_value)  wait_kids();     \
                                kill_kids();        \
                                free_mem();         \
                                free_sysVar();      \
                                exit(exit_value)

#define EXIT_PROCEDURE_USER(exit_value) free_mem_user();         \
                                free_sysVar_user();      \
                                exit(exit_value)
#define ERROR_MESSAGE(err_mex) fprintf(stderr, "%s%s Error: %s -> | file: %s | line: %d | proc_pid: %d | \n (-) %d\n(-)%s\n %s\n", COLOR_RESET_ANSI_CODE, COLOR_RED_ANSI_CODE,err_mex, __FILE__, __LINE__ ,getpid(), errno, strerror(errno),COLOR_RESET_ANSI_CODE)

#define ERROR_EXIT_SEQUENCE_MAIN(str) ERROR_MESSAGE(str);\
                            EXIT_PROCEDURE_MAIN(EXIT_FAILURE)
#define ERROR_EXIT_SEQUENCE_USER(str) ERROR_MESSAGE(str); \
                            EXIT_PROCEDURE_USER(EXIT_FAILURE)
/*  FUNCTION TO PROTECT EXIT SEQUENCE MAIN*/
void wait_kids();

void kill_kids();

void free_mem();

void free_sysVar();

/*  FUNCTION TO PROTECT EXIT SEQUENCE USER*/
void free_mem_user();
void free_sysVar_user();
#endif /*DGT_SO_PROJECT_21_22_ERRORS_H*/
