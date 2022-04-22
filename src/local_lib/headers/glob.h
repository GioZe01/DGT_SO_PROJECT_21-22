#ifndef DGT_SO_PROJECT_21_22_GLOB_H
#define DGT_SO_PROJECT_21_22_GLOB_H

#include <unistd.h>
#include "sys/types.h"
/*  EXCVE PATH   */
#define PATH_TO_USER "build/bin/user"
#define PATH_TO_NODE "build/bin/node"
#define PATH_TO_NODE_TP "build/bin/node_tp"

/*  NUM LIMIT OF PROCS */

/*! \def USERS_MAX
 * \brief The number of processes that can be created
 */
#define USERS_MAX 1000
/*! \def NODES_MAX
 * \brief The number of processes that can be created
 */
#define NODES_MAX 100

/*  SINCRONIZATION KEY values from 60 to 70*/

/*! \def SEMAPHORE_SINC_KEY_START
 * \brief The first key for the sincronization semaphore sincronization
 */
#define SEMAPHORE_SINC_KEY_START 0x60
/*! \def SEMAPHORE_MASTER_BOOK_ACCESS_KEY
 * \brief The key for the masterbook semaphore for single block access
 */
#define SEMAPHORE_MASTER_BOOK_ACCESS_KEY 0x61
/*! \def SEMAPHORE_MASTER_BOOK_TO_FILL_KEY
 * \brief The key for the masterbook semaphore for to_fill shm param
 */
#define SEMAPHORE_MASTER_BOOK_TO_FILL_KEY 0X62
/*  QUEUE KEY  values from 71 to 80*/

/*! \def MASTER_QUEUE_KEY
 * \brief The key for the master report queue
 */
#define MASTER_QUEUE_KEY 0x71
/*! \def USERS_QUEUE_KEY
 * \brief The key for the user report queue
 */
#define USERS_QUEUE_KEY 0x72
/*! \def NODE_QUEUE_KEY
 * \brief The key for the node report queue
 */
#define NODES_QUEUE_KEY 0x73

/*  SHARED MEM. KEY values from 81 to 90*/
/*! \def SHM_CONFIGURATION
 * \brief The key for the shm dedicated to the configuration
 */
#define SHM_CONFIGURATION 0x81
/*! \def SHM_MASTER_BOOK
 * \brief The key for the shm dedicated to the master book
 */
#define MASTER_BOOK_SHM_KEY 0x82

/*  COLOR DEFINITION    */
/*! \def COLOR_RED_ANSI_CODE
 * \brief The color red in ansi code
 */
#define COLOR_RED_ANSI_CODE "\x1b[31m"
/*! \def COLOR_GREEN_ANSI_CODE
 * \brief The color green in ansi code
 */
#define COLOR_GREEN_ANSI_CODE "\x1b[32m"
/*! \def COLOR_YELLOW_ANSI_CODE
 * \brief The color yellow in ansi code
 */
#define COLOR_YELLOW_ANSI_CODE "\x1b[33m"
/*! \def COLOR_BLUE_ANSI_CODE
 * \brief The color blue in ansi code
 */
#define COLOR_BLUE_ANSI_CODE    "\x1b[34m"
/*! \def COLOR_MAGENTA_ANSI_CODE
 * \brief The color magenta in ansi code
 */
#define COLOR_WHITE_ANSI_CODE "\x1b[37m"
/*! \def COLOR_RESET_ANSI_CODE
 * \brief The color reset in ansi code
 */
#define COLOR_RESET_ANSI_CODE "\x1b[0m"
/*AT COMPILATION TIME DEFINITION*/
#ifndef SO_REGISTRY_SIZE
#define SO_REGISTRY_SIZE 100
#endif
#ifndef SO_BLOCK_SIZE
#define SO_BLOCK_SIZE 10
#endif
/*  GENERAL UTILITY MACROS*/
#define REALLOC_MARGIN 10 /**< delta in wich is not usefull to realloc TODO: make a function that calculate it*/
#define MAX_FAILURE_SHM_LOADING 1000 /**< max number of times that the shm loading can fail*/
#define MAX_UNSED_CICLE_OF_NODE_PROC 10000000 /**< max number of times that the node process can be unused*/
#define MAX_WAITING_TIME_FOR_UPDATE 10000 /**< max number of times for updating a specific sys call*/
#define MAX_FAILURE_SHM_BOOKMASTER_LOCKING 10 /**< max number of times that the shm locking can fail*/

/*SIMULATION END TYPES*/
/*! \enum SIMULATION_END_TYPE
 * \brief The type of the simulation that can occured
 */
enum SIMULATION_END_TYPE{
    SIMULATION_END_BY_USER, /**< The simulation is ended by the user*/
    SIMULATION_END_BY_TIME, /**< The simulation is ended by the time*/
    SIMULATION_END_BY_SO_REGISTRY_FULL, /**< The simulation is ended by the registry full*/
    SIMULATION_END_BY_NO_PROC_RUNNING, /**< The simulation is ended by the no proc running*/
    SIMULATION_END_PROPERLY_TERMINATED, /**< The simulation is ended by the properly terminated*/
};

#endif /*DGT_SO_PROJECT_21_22_GLOB_H*/
