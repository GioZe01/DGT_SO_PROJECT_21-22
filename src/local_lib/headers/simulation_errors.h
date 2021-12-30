

#ifndef DGT_SO_PROJECT_21_22_ERRORS_H
#define DGT_SO_PROJECT_21_22_ERRORS_H

#define EXIT_PROCEDURE(exit_value)  wait_kids();     \
                                kill_kids();        \
                                free_mem();         \
                                free_sysVar();      \
                                exit(exit_value)
#define ERROR_MESSAGE(err_mex) fprintf(stderr, "%s%s := Errore -> | file: %s | line: %s | proc_pid: %d | \n (-) %s\n (-){%d} %s\n%s\n", COLOR_RESET_ANSI_CODE, COLOR_RED_ANSI_CODE, __FILE__, __LINE_ ,getpid(), errno, strerror(errno),COLOR_RESET_ANSI_CODE)
#define MESSAGE_ERROR_FILE_CONF(str) ERROR_MESSAGE(("Errore nella lettura del file di configurazione: %s", &str))


void wait_kids();

void kill_kids();

void free_mem();

void free_sysVar();
#endif /*DGT_SO_PROJECT_21_22_ERRORS_H*/
