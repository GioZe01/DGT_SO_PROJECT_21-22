/**
 * \file simulation_errors.h
 * \brief Macros for the procedure of ending the different types of processes in case of error.
 * Contains macros and definition of functions.
 * \warning The functions must be implemented directly in the file where they are used.
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_SIMULATION_ERRORS_H
#define DGT_SO_PROJECT_21_22_SIMULATION_ERRORS_H

#include <string.h>
#include <errno.h>
#include "glob.h"
/**
 * \brief Macro for the procedure of ending the main process in case of error.
 */
#define EXIT_PROCEDURE_MAIN(exit_value) wait_kids();     \
                                kill_kids();        \
                                free_mem();         \
                                free_sysVar();      \
                                exit(exit_value)
/**
 * \defgroup EXIT PROCEDURES
 * \brief Macros for the procedure of ending the child process in case of error.
 */
/**@{*/
#define EXIT_PROCEDURE_USER(exit_value) free_mem_user();         \
                                free_sysVar_user();      \
                                exit(exit_value)

#define EXIT_PROCEDURE_NODE(exit_value) free_mem_node();         \
                                free_sysVar_node();      \
                                exit(exit_value)
/** @}*/
/**
 * Macro for printing the error message given.
 */
#define ERROR_MESSAGE(err_mex)fprintf(stderr, "%s%s Error: %s -> | file: %s | line: %d | proc_pid: %d | \n (-) %d\n(-)%s\n %s\n", COLOR_RESET_ANSI_CODE, COLOR_RED_ANSI_CODE,err_mex, __FILE__, __LINE__ ,getpid(), errno, strerror(errno),COLOR_RESET_ANSI_CODE)

/**
 * \defgroup ERROR EXIT SEQUENCES
 * \brief Macros for the procedure of ending the different types of processes in case of error.
 */
/**@{*/
#define ERROR_EXIT_SEQUENCE_MAIN(str) ERROR_MESSAGE(str);\
                            EXIT_PROCEDURE_MAIN(EXIT_FAILURE)
#define ERROR_EXIT_SEQUENCE_USER(str) ERROR_MESSAGE(str); \
                            EXIT_PROCEDURE_USER(EXIT_FAILURE)

#define ERROR_EXIT_SEQUENCE_NODE(str) ERROR_MESSAGE(str); \
                            EXIT_PROCEDURE_NODE(EXIT_FAILURE)
/** @}*/
/*  FUNCTIONS TO PROTECT EXIT SEQUENCE MAIN*/

void wait_kids();

void kill_kids();

void free_mem();

void free_sysVar();

/*  FUNCTIONS TO PROTECT EXIT SEQUENCE USER*/
void free_mem_user();

void free_sysVar_user();

/*  FUNCTIONS TO PROTECT EXIT SEQUENCE NODE*/
void free_mem_node();

void free_sysVar_node();

#endif /*DGT_SO_PROJECT_21_22_ERRORS_H*/
