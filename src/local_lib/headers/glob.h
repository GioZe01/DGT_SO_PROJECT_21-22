

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
/*  SINCRONIZATION KEY values from 60 to 70*/
#define SEMAPHORE_SINC_KEY_START 0x60
#define SEMAPHORE_MASTER_BOOK_ACCESS_KEY 0x61
/*  QUEUE KEY  values from 71 to 80*/
#define MASTER_QUEUE_KEY 0x71
#define USERS_QUEUE_KEY 0x72
#define NODES_QUEUE_KEY 0x73
/*  SHARED MEM. KEY values from 81 to 90*/
#define SHM_CONFIGURATION 0x81
#define MASTER_BOOK_SHM_KEY 0x82
/*  COLOR DEFINITION    */
#define COLOR_RED_ANSI_CODE "\x1b[31m"
#define COLOR_GREEN_ANSI_CODE "\x1b[32m"
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
