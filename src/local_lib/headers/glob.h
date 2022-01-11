

#ifndef DGT_SO_PROJECT_21_22_GLOB_H
#define DGT_SO_PROJECT_21_22_GLOB_H
#include <unistd.h>
#include "sys/types.h"
/*  EXVE PATH   */
#define PATH_TO_USER "build/bin/user"
/*  SINCRONIZATION KEY  */
#define SEMAPHORE_SINC_KEY_START 0x76
/*  QUEUE ID    */
#define QUEUE_SHIFT_ID 0x77/*TODO:vedere se ha senso*/
/*  SHARED MEM. */
#define SHM_USERS_PROC_KEY 0x78
/*  COLOR DEFINITION    */
#define COLOR_RED_ANSI_CODE "\x1b[31m"
#define COLOR_GREEN_ANSI_CODE "\x1b[32m" /* Usato per printing debugging mode per semafori*/
#define COLOR_YELLOW_ANSI_CODE "\x1b[33m"
#define COLOR_BLUE_ANSI_CODE    "\x1b[34m"
#define COLOR_WHITE_ANSI_CODE "\x1b[37m"
#define COLOR_RESET_ANSI_CODE "\x1b[0m"

void int_to_hex(pid_t pid, char * buffer);

#endif /*DGT_SO_PROJECT_21_22_GLOB_H*/
