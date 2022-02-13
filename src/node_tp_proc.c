/* Handle a single node transaction queue, keep loading the transactions into a transaction pool,
 * as node_proc is data status is rappresented via the ADT structure "node".
 * -> responsability: is responsable of mantaining the tranzaction pool and updating the node_tp_shm
 *                    and advice the user in case of max transaction in tp. and
 *                    send back the failure. Also sending an advice to the proc_master
 *                    of his termination in case of failure or timeout.
 * */

/*  Standard Library */
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


/*  Local Library  */
#include "local_lib/headers/glob.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/simulation_errors.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/node_transactor.h"
#include "local_lib/headers/user_msg_report.h"
#include "local_lib/headers/node_msg_report.h"
#include "local_lib/headers/conf_shm.h"
#include "local_lib/headers/book_master_shm.h"
#include "local_lib/headers/master_msg_report.h"
#ifdef DEBUG

#ifdef DEBUG_NODE_TP
#include "local_lib/headers/debug_utility.h"
#else
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif
#endif

/* Support Function*/
/**
 * \brief Acquire the semaphore_id related to the node
 * In particular:
 * - semaphore_start_id
 * - semaphore_node_tp_shm_access
 */
void acquire_semaphore_ids(void);
/**
 * Check the argc and argv to match with project specification and load so_tp_size
 * @param argc number of argument given
 * @param argv pointer to a char list of params given
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool check_arguments(int argc, char const *argv[]);

/**
 * Connects to the differents queues: master, node's and user's
 * */
void connect_to_queues(void);

/**
 * handler of the signal
 * @param signum type of signal to be handled
 */
void signals_handler(int signum);
int state;
struct node current_node_tp;
int main(int argc, char const *argv[]){
    DEBUG_MESSAGE("NODE TP PROCESS STARTED");
    struct sigaction sa;
    sigset_t sigmask;
    DEBUG_MESSAGE("NODE TP PROCESS SET TO INIT");
    current_node_tp.exec_state = PROC_STATE_INIT;
    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
    if(check_arguments(argc,argv)==TRUE){
    }
}


Bool check_arguments(int argc, char const *argv[]) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
    if (argc < 2) {
        ERROR_EXIT_SEQUENCE_NODE_TP("MISSING ARGUMENT");
    }
    int tp_size = atoi(argv[1]);
    if(tp_size <=0){
        ERROR_EXIT_SEQUENCE_NODE_TP("TP_SIZE IS <= 0. NOT ACCEPTED");
    }
    current_node_tp.tp_size=tp_size;
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}
