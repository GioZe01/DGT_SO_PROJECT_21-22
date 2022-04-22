/* Handle a single node transaction queue, keep loading the transactions into a transaction pool,
 * as node_proc is data status is rappresented via the ADT structure "node".
 *
 * -> responsability: is responsable of mantaining the tranzaction pool and updating the node_tp_shm
 *                    and advice the user in case of max transaction in tp by sending back the
 *                    failure. Also sending an advice to the proc_master
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
#include "local_lib/headers/node_tp_shm.h"
#include "local_lib/headers/debug_utility.h"

/* Support Function*/

/* Advice the master porc via master_message queue by sending a master_message
 * @param termination_type MSG_REPORT_TYPE type process termination occured
 */
void advice_master_of_termination(long termination_type);

/**
 * @brief Acquire the semaphore_id related to the node
 * In particular:
 * - semaphore_start_id
 * - semaphore_node_tp_shm_access
 */
void acquire_semaphore_ids(void);

/**
 * @brief Attach the current node to the related shms
 * Specificcaly:
 *  -node_tp_shm
 */
void attach_to_shms(void);

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

/*
 * Load the block into the shm_node_tp if semval is set to 0
 * @return TRUE if all operetaion succeded, false otherwise
 * */
Bool load_block_to_shm(void);

/**
 * handler of the signal
 * @param signum type of signal to be handled
 */
void signals_handler(int signum);
/* TODO: refactoring of comments of the following process transaction*/
/**
 * If it finds node msg of type NODE_TRANSACTION it proccess them
 * and make the aknowledgement
 * @param msg_rep the messagge to be loaded if present in the queue
 * */
void process_node_transaction(struct node_msg *msg_rep);

/*
 * If it finds node msg of type TRANSACTION_TYPE it process them
 * and make the aknowledgement adding them to the transaction_pool and the transaction_block if
 * need
 * @param msg_rep the messagge to be loaded if present in the queue
 * */
void process_simple_transaction_type(struct node_msg *msg_rep);

/**
 * Set the handler for signals of the current main_proc
 * @param sa  describe the type of action to performed when a signal arrive
 */
void set_signal_handlers(struct sigaction sa, sigset_t sigmask);

/*
 * Get new SO_BLOCK_SIZE transactions from the transaction_list in the current_node_tp
 * */
void update_block(void);

/* SysVar */
int parent_id = -1;                     /* Id of the start semaphore arrays, ref. for shm_key*/
int sem_start_id = -1;                  /*Id of the start semaphore arrays for sinc*/
int sem_node_tp_id = -1;                /* Id of the semaphore for accesing node_tp_shm*/
int queue_node_id = -1;                 /* Identifier of the node queue id */
int queue_user_id = -1;                 /* Identifier of the user queue id*/
int queue_master_id = -1;               /* Identifier of the master queue id*/
int node_tp_end = 0;                    /* For values different from 0 the node proc must end*/
int last_signal;                        /* Last signal received*/
struct node current_node_tp;            /* Current rappresentation of the node_tp */
struct node_block *shm_node_tp;         /* Ref to the shm for the node_tp_shm */
struct shm_conf *shm_conf_pointer_node; /* Ref to the shm for configuration of the node*/

int main(int argc, char const *argv[])
{
    DEBUG_MESSAGE("NODE TP PROCESS STARTED");
    int failure_shm = 0;
    DEBUG_MESSAGE("NODE TP PROCESS SET TO INIT");
    current_node_tp.exec_state = PROC_STATE_INIT;
    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
    node_proc_tp_create(&current_node_tp, getpid(), current_node_tp.type.tp.tp_size);
    if (check_arguments(argc, argv) == TRUE)
    {
        struct node_msg msg_rep;
        struct sigaction sa; /*Structure for handling signals */
        sigset_t sigmask;
        int is_unsed_node = 0;
        set_signal_handlers(sa, sigmask);
        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/
        /*---------------------------*/
        /*  SHM CONNECTION           */
        /*---------------------------*/
        attach_to_shms();
        /*---------------------------*/
        /*  SEMAPHORES ACQUISITION   */
        /*---------------------------*/
        acquire_semaphore_ids();

        /*-----------------------*/
        /*  CONNECTING TO QUEUES */
        /*-----------------------*/
        connect_to_queues();

        /*---------------------------*/
        /*  SEMAPHORES SINC..        */
        /*---------------------------*/
        DEBUG_MESSAGE("NODE READY, ON START_SEM");
        if (semaphore_wait_for_sinc(sem_start_id, 0) < 0)
        {
            ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO WAIT FOR START");
        }
        current_node_tp.exec_state = PROC_STATE_RUNNING;
        while (node_tp_end != 1 && failure_shm < MAX_FAILURE_SHM_LOADING)
        {
#ifdef DEBUG_NODE_TP
            struct msqid_ds info;
            if (msgctl(queue_node_id, IPC_STAT, &info) < 0)
            {
                ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO COMUNICATE WITH THE QUEUES");
            }
            if (info.msg_qnum > 0)
            {
                printf("\n{DEBUG_NODE_TP} %d := NUMBER OF TRANSACTION IN LIST: %d | NUMBER OF MESSAGES : %ld\n", getpid(), get_num_transactions(current_node_tp.transactions_list), info.msg_qnum);
            }
#endif
            process_node_transaction(&msg_rep);
            process_simple_transaction_type(&msg_rep);
            if (msg_rep.sender_pid == -1)
            {
                is_unsed_node++;
            }
            /*
#ifdef DEBUG_NODE_TP
            if (msg_rep.sender_pid >=0 ){
                node_msg_print(&msg_rep);
                queue_print(current_node_tp.transactions_list);
                struct timespec print_waiting_time;
                print_waiting_time.tv_sec = 0;
                print_waiting_time.tv_nsec = 10000;
                nanosleep(&print_waiting_time, (void *)NULL);
            }
#endif*/
            if (get_num_transactions(current_node_tp.transactions_list) >= SO_BLOCK_SIZE &&
                load_block_to_shm() == FALSE)
            {
                failure_shm++;
            }
        }
        if (failure_shm > MAX_FAILURE_SHM_LOADING)
        {
            ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO READ DATA FROM NODE_TP_SHM");
        }
        if (is_unsed_node >= MAX_UNSED_CICLE_OF_NODE_TP)
        {
            advice_master_of_termination(UNUSED_PROC);
            ERROR_EXIT_SEQUENCE_NODE_TP("UNUSED NODE TP PROC");
        }
    }
    advice_master_of_termination(TERMINATION_END_CORRECTLY);
    EXIT_PROCEDURE_NODE_TP(0);
}

Bool check_arguments(int argc, char const *argv[])
{
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
    if (argc < 2)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("MISSING ARGUMENT");
    }
    int tp_size = atoi(argv[1]);
    int node_id = atoi(argv[2]);
    parent_id = atoi(argv[3]);
    if (tp_size <= 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("TP_SIZE IS <= 0. NOT ACCEPTED");
    }
    current_node_tp.type.tp.tp_size = tp_size;
    current_node_tp.node_id = node_id;
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}

void advice_master_of_termination(long termination_type)
{
    struct master_msg_report termination_report;
    if (master_msg_send(queue_master_id, &termination_report, termination_type, NODE_TP, current_node_tp.pid,
                        current_node_tp.exec_state, TRUE, -1) < 0)
    {
        char *error_string = strcat("IMPOSSIBLE TO ADVICE MASTER OF : %s", from_type_to_string(termination_type));
        ERROR_MESSAGE(error_string);
    }
}

void acquire_semaphore_ids(void)
{
    sem_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
    if (sem_start_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO OBTAIN ID OF START SEM");
    }
    if (semaphore_lock(sem_start_id, 0) < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
    }
    sem_node_tp_id = semget(current_node_tp.node_id, 1, 0);
    if (sem_node_tp_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO RETRIEVE");
    }
}

void process_simple_transaction_type(struct node_msg *msg_rep)
{
    if (node_msg_receive(queue_node_id, msg_rep, current_node_tp.node_id) == 0)
    {
        DEBUG_MESSAGE("NODE TRANSACTION TYPE RECEIVED");
        if (get_num_transactions(current_node_tp.transactions_list) < current_node_tp.type.tp.tp_size)
        {
            queue_append(current_node_tp.transactions_list, msg_rep->t);
        }
        else
        { /*TP_SIZE FULL*/
            struct user_msg *u_msg_rep = (struct user_msg *)malloc(sizeof(struct user_msg));
#ifdef DEBUG_NODE_TP
            DEBUG_ERROR_MESSAGE("NODE TRANSACTION FAILED");
#endif
            u_msg_rep->t.t_type = TRANSACTION_FAILED;
            int queue_id_user_proc = get_queueid_by_pid(shm_conf_pointer_node, msg_rep->sender_pid, TRUE);
            if (queue_id_user_proc < 0)
            {
                ERROR_MESSAGE("ILLIGAL PID INTO TRANSACTION, NO PIDS FOUND");
                return;
            }
            user_msg_snd(queue_user_id, u_msg_rep, MSG_TRANSACTION_FAILED_TYPE, &msg_rep->t, current_node_tp.pid, TRUE, queue_id_user_proc);
        }
    }
    else
    {
        msg_rep->sender_pid = -1;
    }
}

void process_node_transaction(struct node_msg *msg_rep)
{
    if (node_msg_receive(queue_node_id, msg_rep, current_node_tp.node_id - MSG_NODE_ORIGIN_TYPE) == 0)
    {
        /*Checking for transaction coming from node*/
        DEBUG_MESSAGE("NODE TRANSACTION RECEIVED");
        /*TODO: Implement incoming transaction from other node*/
    }
    else
    {
        msg_rep->sender_pid = -1;
    }
}

Bool load_block_to_shm(void)
{
    int sem_val = semctl(sem_node_tp_id, 0, GETVAL);
    if (sem_val == IS_EMPTY && get_num_transactions(current_node_tp.transactions_list) >= SO_BLOCK_SIZE)
    {
        update_block();
        if (semctl(sem_node_tp_id, 0, SETVAL, FULL) < 0)
        {
            ERROR_MESSAGE("IMPOSSIBLE TO SET NODE TP SEM TO FULL");
            return FALSE;
        }
    }
    else if (sem_val < 0)
    {
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIVE VAL FROM TP SHM SEMAPHORE");
        return FALSE;
    }
    return TRUE;
}

void update_block(void)
{
    int i = 0;
    for (; i < SO_BLOCK_SIZE; i++)
    {
        shm_node_tp->block_t[i] = queue_head(current_node_tp.transactions_list);
        queue_remove_head(current_node_tp.transactions_list);
    }
}

void set_signal_handlers(struct sigaction sa, sigset_t sigmask)
{
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signals_handler;
    sa.sa_mask = sigmask;
    if (sigaction(SIGUSR2, &sa, NULL) < 0 ||
        sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("ERROR SETTTING SIGNAL HANDLERS");
    }
}

void signals_handler(int signum)
{
    DEBUG_SIGNAL("SIGNAL RECEIVED ", signum);
    last_signal = signum;
    struct master_msg_report msg;
    switch (signum)
    {
    case SIGINT:
        alarm(0); /*pending alarm removed*/
        current_node_tp.exec_state = PROC_STATE_TERMINATED;
        advice_master_of_termination(SIGNALS_OF_TERM_RECEIVED);
        EXIT_PROCEDURE_NODE_TP(0);
    case SIGALRM:
        break;
    case SIGUSR2:
        master_msg_send(queue_master_id, &msg, INFO_BUDGET, NODE_TP, current_node_tp.pid, current_node_tp.exec_state, TRUE, -1);
#ifdef DEBUG_NODE_TP
        DEBUG_NOTIFY_ACTIVITY_DONE("{DEBUG_NODE_TP}:= REPLIED TO MASTER DONE");
#endif
        break;
    default:
        break;
    }
}

void connect_to_queues(void)
{
    queue_node_id = msgget(NODES_QUEUE_KEY, 0600);
    if (queue_node_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO CONNECT TO NODE MESSAGE QUEUE");
    }
    queue_user_id = msgget(USERS_QUEUE_KEY, 0600);
    if (queue_user_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO CONNECT TO USER QUEUE");
    }
    queue_master_id = msgget(MASTER_QUEUE_KEY, 0600);
    if (queue_master_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO CONNECT TO MASTER QUEUE");
    }
}

void free_mem_node_tp()
{
    free_node(&current_node_tp);
}

void free_sysVar_node_tp()
{
    int semaphore_start_value;
    /* If the process is in INIT STATE his termination can block the whole simulation. So it need to
     * lock the sart semaphore.
     * If the semaphore_start_id is impossible to retrive ( = -1) nothing can be done.
     * */
    if (current_node_tp.exec_state == PROC_STATE_INIT && sem_start_id >= 0)
    {
        semaphore_start_value = semctl(sem_start_id, 0, GETVAL);
        if (semaphore_start_value < 0)
            ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0 && (semaphore_lock(sem_start_id, 0) < 0))
        {
            ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }
    current_node_tp.exec_state = PROC_STATE_TERMINATED;
}
void attach_to_shms(void)
{
    DEBUG_NOTIFY_ACTIVITY_RUNNING("{NODE_TP_PROC}:= ATTACHING TO SHM ...");
    int shm_conf_id = -1; /* id to the shm_conf*/
    shm_conf_id = shmget(SHM_CONFIGURATION, sizeof(struct shm_conf), 0600);
    if (shm_conf_id < 0)
    {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO ACCESS SHM CONF");
    }
    shm_conf_pointer_node = shmat(shm_conf_id, NULL, 0);
    if (shm_conf_pointer_node == (void *)-1)
    {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO CONNECT TO SHM CONF");
    }
    int shm_tp_id = -1;
    shm_tp_id = shmget(parent_id, sizeof(struct node_block), 0600);
    if (shm_tp_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO CREATE NODE TP_SHM");
    }
    shm_node_tp = shmat(shm_tp_id, NULL, 0);
    if (shm_node_tp == (void *)-1)
    {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE_TP("IMPOSSIBLE TO CONNECT TO THE NODE_TP SHM");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("{DEBUG NODE PROC}:= ATTACHING TO SHM DONE");
}
