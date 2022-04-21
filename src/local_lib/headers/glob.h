#ifndef DGT_SO_PROJECT_21_22_GLOB_H
#define DGT_SO_PROJECT_21_22_GLOB_H

#include <unistd.h>
#include "sys/types.h"
/*  EXCVE PATH   */
#define PATH_TO_USER "build/bin/user"
#define PATH_TO_NODE "build/bin/node"
#define PATH_TO_NODE_TP "build/bin/node_tp"
/*  NUM LIMIT OF PROCS */
#define USERS_MAX 1000 /*Max users that can be saved in shm*/
#define NODES_MAX 100    /*Max nodes that can be saved in shm*/
/*  SINCRONIZATION KEY values from 60 to 70*/
#define SEMAPHORE_SINC_KEY_START 0x60
#define SEMAPHORE_MASTER_BOOK_ACCESS_KEY 0x61 /*For single block access*/
#define SEMAPHORE_MASTER_BOOK_TO_FILL_KEY 0X62 /*For to_fill shm param look for def into shm_masterbook*/
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
/*  GENERAL UTILITY MACROS*/
#define REALLOC_MARGIN 10 /*delta in wich is not usefull to realloc TODO: make a function that calculate it*/
#define MAX_FAILURE_SHM_LOADING 1000
#define MAX_UNSED_CICLE_OF_NODE_PROC 10000000
#define MAX_WAITING_TIME_FOR_UPDATE 10000
#define MAX_FAILURE_SHM_BOOKMASTER_LOCKING 10

/*SIMULATION END TYPES*/
enum simulation_end_type {
    SIMULATION_END_BY_USER,
    SIMULATION_END_BY_TIME,
    SIMULATION_END_BY_SO_REGISTRY_FULL,
    SIMULATION_END_BY_NO_PROC_RUNNING,
    SIMULATION_END_PROPERLY_TERMINATED,
};

#endif /*DGT_SO_PROJECT_21_22_GLOB_H*/
