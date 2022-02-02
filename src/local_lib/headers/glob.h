

#ifndef DGT_SO_PROJECT_21_22_GLOB_H
#define DGT_SO_PROJECT_21_22_GLOB_H

#include <unistd.h>
#include "sys/types.h"
/*  EXCVE PATH   */
#define PATH_TO_USER "build/bin/user"
#define PATH_TO_NODE "build/bin/node"
/*  NUM LIMIT OF PROCS */
#define USERS_MAX 1000 /*Max users that can be saved in shm*/
#define NODES_MAX 100    /*Max nodes that can be saved in shm*/
/*  SINCRONIZATION KEY  */
#define SEMAPHORE_SINC_KEY_START 0x76
/*  QUEUE KEY    */
#define MASTER_QUEUE_KEY 0x77
#define USERS_QUEUE_KEY 0x78
#define NODES_QUEUE_KEY 0x79
/*  SHARED MEM. KEY*/
#define SHM_CONFIGURATION 0x74
#define MASTER_BOOK_SHM_KEY 0x75
/*  COLOR DEFINITION    */
#define COLOR_RED_ANSI_CODE "\x1b[31m"
#define COLOR_GREEN_ANSI_CODE "\x1b[32m" /* Usato per printing debugging mode per semafori*/
#define COLOR_YELLOW_ANSI_CODE "\x1b[33m"
#define COLOR_BLUE_ANSI_CODE    "\x1b[34m"
#define COLOR_WHITE_ANSI_CODE "\x1b[37m"
#define COLOR_RESET_ANSI_CODE "\x1b[0m"
/*AT COMPILATION TIME DEFINITION*/
#ifndef SO_REGISTRY_SIZE
#define SO_REGISTRY_SIZE 100
#endif
#ifndef SO_BLOCK_SIZE
#define SO_BLOCK_SIZE 10
#endif
/*  GENERAL UTILITY MACRO*/
#define REALLOC_MARGIN 10 /*delta in wich is not usefull to realloc TODO: make a function that calculate it*/

#endif /*DGT_SO_PROJECT_21_22_GLOB_H*/
