/*  Standard Library  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
/*  Sys Library */
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
/*  Local Library */
#include "local_lib/headers/simulation_errors.h"
#include "local_lib/headers/transaction_list.h"
#include "local_lib/headers/user_transaction.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/user_msg_report.h"
#include "local_lib/headers/node_msg_report.h"
#include "local_lib/headers/master_msg_report.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/conf_shm.h"
#include "local_lib/headers/debug_utility.h"
#include "local_lib/headers/gt_sig_handler.h"

/*  Support functions*/

/* Advice the master porc via master_message queue by sending a master_message
 * @param termination_type MSG_REPORT_TYPE type process termination occured
 */
void advice_master_of_termination(long termination_type);

/**
 * Connects to the differents queues: master, node's and user's
 * */
void connect_to_queues(void);

/**
 * handler of the signal
 * @param signum type of signal to be handled
 */
void signals_handler_user(int signum);

/**
 * Check the argc and argv to match with project specification
 * @param argc number of argument given
 * @param argv pointer to a char list of params given
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool check_argument(int argc, char const *argv[]);

/**
 * Check for the presence of a confirmed transaction and eventually handles it
 * @return
 */
Bool check_for_transactions_confirmed(void);

/**
 * Check for the presence of a failed transaction and eventually handles it
 * @return
 */
Bool check_for_transactions_failed(void);

/**
 * Set the handler for signals of the current user_proc
 * @param sa describe the type of action to be performed when a signal arrive
 * @param sigmask the mask to be applied
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool set_signal_handler_user(struct sigaction sa);

/**
 * @brief Read the conf file present in te project dir
 * load the configuration directly in the struct conf configuration that is a SysVar
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool read_conf();

/**
 * @brief Send to the node all the transaction that still need to be processed and pop them
 * from the transaction_list
 * with range block size if cashing is active
 * @return -1 in case of FAILURE. 0 otherwise
 */
int send_to_node(void);

/**
 * Make the shm_conf_pointer points to the correct conf shm
 */
void attach_to_shm_conf(void);

/**
 * generate the transactions, and take knowledge of usr_msg int usr_msg_queue
 */
void generating_transactions(void);

/**
 * Check for incoming transactions with him as receiver
 */
Bool getting_richer(void);

/*  SysV  */
int semaphore_start_id = -1;          /*Id of the start semaphore arrays for sinc*/
int queue_node_id = -1;               /* Identifier of the node queue id */
int queue_user_id = -1;               /* Identifier of the user queue id*/
int queue_master_id = -1;             /* Identifier of the master queue id*/
int user_id = -1;                     /*Id of the current user into the snapshots vectors*/
sigset_t current_mask;                /* Current mask of the user*/
struct user_transaction current_user; /* Current representation of the user*/
struct conf configuration;            /* Configuration File representation */
struct shm_conf *shm_conf_pointer;    /* Ref to the shm fir configuration of the user*/

int main(int arc, char const *argv[]) {
    DEBUG_MESSAGE("USER PROCESS STARTED");
    struct sigaction sa;
    int start_sem_value;
    srand(getpid());

    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
#ifdef DEBUG_USER
    DEBUG_MESSAGE("USER STATE IS SET TO INIT");
#endif
    current_user.exec_state = PROC_STATE_INIT;

    /*  VARIABLE INITIALIZATION */
    read_conf();
    user_create(&current_user, configuration.so_buget_init, getpid(), (Balance) &calc_balance,
                (CalcCashFlow) &update_cash_flow);
    if (check_argument(arc, argv) && set_signal_handler_user(sa)) {

        /*--------------------------------------*/
        /*  CONNECTING TO THE USER REPORT QUEUE */
        /*--------------------------------------*/
        connect_to_queues();

        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/

        /*------------------------------------*/
        /*  WAITING START SEMAPHORE TO UNLOCK *
         * -----------------------------------*
         * INFO:                              *
         * unlock is done if sem hasn't 0 as  *
         * value                              */
        /*------------------------------------*/
        /*TODO: need a semafore for reading into the message queue*/
        semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
        if (semaphore_start_id < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN ID OF START SEM.");
        }
        start_sem_value = semctl(semaphore_start_id, 0, GETVAL);
        if (start_sem_value < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN INFO FROM START SEM.");
        }
        if (start_sem_value != 0 && semaphore_lock(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
        }
        DEBUG_MESSAGE("USER READY, WAITING FOR SEMAPHORE TO FREE");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("ERROR DURING WAITING START_SEMAPHORE UNLOCK");
        }
        current_user.exec_state = PROC_STATE_RUNNING;


        /*-------------------------*/
        /*  SHARED MEM  CONFIG     */
        /*-------------------------*/
        attach_to_shm_conf();

        /****************************************
         *      GENERATION OF TRANSACTION FASE *
         * **************************************/
        while (current_user.exec_state == PROC_STATE_RUNNING) {
            generating_transactions();
            getting_richer();
        }
        /*TODO: check for remaining transaction confirmed*/
        current_user.exec_state = PROC_STATE_TERMINATED;
        advice_master_of_termination(TERMINATION_END_CORRECTLY);
        EXIT_PROCEDURE_USER(0);
    }

    ERROR_EXIT_SEQUENCE_USER("CREATION OF USER_PROC FAILED DUE TO: Arg or Signal handler creation failure");
}

Bool check_argument(int argc, char const *argv[]) {
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
#endif
    if (argc < 2) {
        ERROR_EXIT_SEQUENCE_USER("MISSING ARGUMENT");
    }
    user_id = atoi(argv[1]);
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
#endif
    return TRUE;
}

void connect_to_queues(void) {
    queue_node_id = msgget(NODES_QUEUE_KEY, 0600);
    if (queue_node_id < 0) {
        ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CONNECT TO NODE MESSAGE QUEUE");
    }
    queue_user_id = msgget(USERS_QUEUE_KEY, 0600);
    if (queue_user_id < 0) {
        ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CONNECT TO USER QUEUE");
    }
    queue_master_id = msgget(MASTER_QUEUE_KEY, 0600);
    if (queue_master_id < 0) {
        ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CONNECT TO MASTER QUEUE");
    }
}

Bool set_signal_handler_user(struct sigaction sa) {
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&current_mask);
    sa.sa_handler = signals_handler_user;
    sa.sa_mask = current_mask;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0 ||
        sigaction(SIGUSR2, &sa, NULL) < 0) {
        ERROR_EXIT_SEQUENCE_USER("ERROR DURING THE CREATION OF THE SIG HANDLER ");
        return FALSE;
    }
    return TRUE;
}

void signals_handler_user(int signum) {
    DEBUG_SIGNAL("SIGNAL RECEIVED", signum);
    struct master_msg_report msg;
    struct Transaction t;
    switch (signum) {
        case SIGINT:
            alarm(0); /* pending alarm removed*/
            current_user.exec_state = PROC_STATE_TERMINATED;
            advice_master_of_termination(SIGNALS_OF_TERM_RECEIVED);
            EXIT_PROCEDURE_USER(0);
            break;
        case SIGALRM: /*    Generate a new transaction  */
            DEBUG_NOTIFY_ACTIVITY_RUNNING("GENERATING A NEW TRANSACTION FROM SIG...");
            if (generate_transaction(&current_user, current_user.pid, shm_conf_pointer) < 0) {
                ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO GENERATE TRANSACTION");
            }
            if (send_to_node() < 0) {
                ERROR_MESSAGE("IMPOSISBLE TO SEND TO THE NODE");
            }
            DEBUG_NOTIFY_ACTIVITY_DONE("GENERATING A NEW TRANSACTION FROM SIG DONE");
            break;
        case SIGUSR2:
            t = create_empty_transaction();
            if (master_msg_send(queue_master_id, &msg, INFO_BUDGET, USER, current_user.pid,
                                current_user.exec_state, TRUE, current_user.budget, &t) < 0) {
                char *error_string = strcat("IMPOSSIBLE TO ADVICE MASTER OF : %s", from_type_to_string(INFO_BUDGET));
                ERROR_MESSAGE(error_string);
            }
            break;
        default:
            break;
    }
}

void advice_master_of_termination(long termination_type) {
    struct master_msg_report termination_report;
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_RUNNING("{DEBUG_USER}:= ADVICING MASTER OF TERMINATION ....");
#endif
    current_user.exec_state = PROC_STATE_TERMINATED;
    struct Transaction t = create_empty_transaction();
    if (master_msg_send(queue_master_id, &termination_report, termination_type, USER, current_user.pid,
                        current_user.exec_state, TRUE, current_user.budget, &t) < 0) {
        char *error_string = strcat("IMPOSSIBLE TO ADVICE MASTER OF : %s", from_type_to_string(termination_type));
        ERROR_MESSAGE(error_string);
    }
#ifdef DEBUG_USER
        DEBUG_NOTIFY_ACTIVITY_DONE("{DEBUG_USER}:= ADVICING MASTER OF TERMINATION DONE");
#endif
    DEBUG_NOTIFY_ACTIVITY_DONE("{DEBUG_USER}:= ADVICING MASTER OF TERMINATION DONE");
}

void free_mem_user() {
    free_user(&current_user);
}

void free_sysVar_user() {
    int semaphore_start_value;
    if (current_user.exec_state == PROC_STATE_RUNNING && semaphore_start_id >= 0) {
        semaphore_start_value = semctl(semaphore_start_id, 0, GETVAL);
        if (semaphore_start_value < 0)
            ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0 && (semaphore_lock(semaphore_start_id, 0) < 0)) {
            ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }
}

Bool read_conf() {
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION...");
#endif
    switch (load_configuration(&configuration)) {
        case 0:
            break;
        case -1:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: MISSING FILE OR EMPTY");
        case -2:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: BROKEN SIMULTATION LOGIC, CHECK CONF. VALUE");
        case -3:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: NOT ENOUGH USERS FOR NODES");
        case -4:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: MIN MAX EXECUTION TIME WRONG");
        case -5:
        ERROR_EXIT_SEQUENCE_USER(" DURING CONF. LOADING: NODE REWARD IS OVER POSSIBILITIES OF USERS");
        default:
            return FALSE;
    }
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_DONE("CONFIGURATION LOADED");
#endif
    return TRUE;
}

int send_to_node(void) {
    if (get_num_transactions(current_user.in_process) > 0) {
#ifdef DEBUG_USER
        DEBUG_NOTIFY_ACTIVITY_RUNNING("SENDING TRANSACTION TO THE NODE...");
#endif
        srand(getpid());
        int node_num = extract_node(shm_conf_pointer->nodes_snapshots[0][0]);
        struct node_msg msg;
        struct Transaction t = queue_head(current_user.in_process);
        if (node_msg_snd(queue_node_id, &msg, MSG_TRANSACTION_TYPE, &t,
                         current_user.pid, TRUE, configuration.so_retry,
                         shm_conf_pointer->nodes_snapshots[node_num][1]) < 0) {
            return -1;
        }
        queue_remove_head(current_user.in_process); /*removed if and only if has been sent*/
#ifdef DEBUG_USER
        node_msg_print(&msg);
        DEBUG_NOTIFY_ACTIVITY_DONE("SENDING TRANSACTION TO THE NODE DONE");
#endif
        return 0;
    } else {
        return -1;
    }
}

void attach_to_shm_conf(void) {
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_RUNNING("ATTACHING TO SHM...");
#endif
    int shm_conf_id; /* id to the shm_conf*/
    shm_conf_id = shmget(SHM_CONFIGURATION, sizeof(struct shm_conf), 0600);
    if (shm_conf_id < 0) {
        ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO ACCESS SHM CONF");
    }
    shm_conf_pointer = shmat(shm_conf_id, NULL, 0);
    if (shm_conf_pointer == (void *) -1) {
        ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO CONNECT TO SHM CONF");
    }
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO SHM DONE");
#endif
}

void generating_transactions(void) {

    struct timespec gen_sleep;
    int failed_gen_trans = 0;
    while (failed_gen_trans < configuration.so_retry && current_user.budget >= 0) {
        block_signal(SIGUSR2, &current_mask);
        getting_richer();
        check_for_transactions_confirmed();
        check_for_transactions_failed();
        int gen_trans_ris = generate_transaction(&current_user, current_user.pid, shm_conf_pointer);
        if (gen_trans_ris == -1) {
            failed_gen_trans++;
#ifdef DEBUG_USER
            ERROR_MESSAGE("IMPOSSIBLE TO GENERATE TRANSACTION");
#endif
        } else if (gen_trans_ris >= 0) {
            gen_sleep.tv_nsec =
                    (rand() % (configuration.so_max_trans_gen_nsec - configuration.so_min_trans_gen_nsec + 1)) +
                    configuration.so_min_trans_gen_nsec;
#ifdef U_CASHING
            /*TODO: make cashing*/
#else
            /*SENDING TRANSACTION TO THE NODE*/
            if (send_to_node() < 0) {
#ifdef DEBUG_USER
                ERROR_MESSAGE("IMPOSSIBLE TO SEND TO THE NODE");
#endif
            } else {
#ifdef DEBUG_USER
                DEBUG_MESSAGE("TRANSACTION SENT TO THE NODE");
#endif
            }

#endif
            nanosleep(&gen_sleep, (void *) NULL);
        }
        unblock_signal(SIGUSR2, &current_mask);
    }

}

Bool check_for_transactions_confirmed(void) {
    struct user_msg msg;
    if (user_msg_receive(queue_user_id, &msg, user_id) == 0) {
        /*Messagge found*/
        current_user.to_wait_transaction--;
        queue_remove(current_user.in_process, msg.t);
        current_user.update_cash_flow(&current_user, msg.t);
        return TRUE;
    }
    return FALSE;
}

Bool check_for_transactions_failed(void) {
    struct user_msg msg;
    if (user_msg_receive(queue_user_id, &msg, (user_id - 2)) == 0) {
        /*Take aknowledgement of transaction falure*/
        current_user.to_wait_transaction--;
        queue_append(current_user.transactions_failed, msg.t);
        queue_remove(current_user.in_process, msg.t);
        current_user.update_cash_flow(&current_user, msg.t);
        return TRUE;
    }
    return FALSE;
}

Bool getting_richer(void) {
    struct user_msg msg;
    if (user_msg_receive(queue_user_id, &msg, user_id - 1) == 0) {
        current_user.update_cash_flow(&current_user, msg.t);
        return TRUE;
    }
    return FALSE;
}


