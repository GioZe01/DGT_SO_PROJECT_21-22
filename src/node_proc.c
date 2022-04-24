#define _GNU_SOURCE
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
#include "local_lib/headers/transaction_list.h"
#include "local_lib/headers/int_condenser.h"

/* Support Functions*/

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
 * Advice the users of the processed transactions, both receivers and senders
 */
void adv_users_of_block(void);

/**
 * handler of the signal
 * @param signum type of signal to be handled
 */
void signals_handler(int signum);

/**
 * @brief Read the conf file present in te project dir
 * load the configuration directly in the struct conf node_configuration that is a SysVar
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool read_conf_node();

/**
 * Set the handler for signals of the current node_proc
 * @param sa describe the type of action to be performed when a signal arrive
 * @param sigmask the mask to be applied
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool set_signal_handler_node(struct sigaction sa, sigset_t sigmask);

/**
 * Check the argc and argv to match with project specification
 * @param argc number of argument given
 * @param argv pointer to a char list of params given
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool check_arguments(int argc, char const *argv[]);

/**
 * @brief Attach the current node to the related shms
 * Specificcaly:
 *  -book_master_shm
 *  -node_tp_shm
 */
void attach_to_shms(void);

/**
 * If node_transaction pool has at list node_block_size of transactions it load them in a transaction block
 * and process them
 * @return -1 in case of failure. 2 in case of pool_size<block_size. 0 otherwise
 */
int process_node_block();

/**
 * Connects to the differents queues: master, node's and user's
 * */
void connect_to_queues(void);

/**
 * load block_size transactions from transaction pool into transaction block of the current node
 * @return TRUE if succeded, FALSE otherwise
 */
Bool load_block(void);

/**
 * @brief Acquire the semaphore_id related to the node
 * Specificcaly:
 * - semaphore_start_id
 * - semaphore_masterbook_id
 * - semaphore_node_tp_shm_access
 */
void acquire_semaphore_ids(void);

/* Lock the shm book_master via masterbook_to_fill param and the cell to fill index.
 * @return FALSE in case of failure, TRUE otherwise
 */
Bool lock_shm_masterbook(void);

/* Lock the to_fill index saved in the shared_memory access
 */
void lock_to_fill_sem(void);

/* Lock the current cell of the masterbook shm in which the node had written the block
 * @param i_cell_block_list index to be blocked
 */
void lock_masterbook_cell_access(int i_cell_block_list);

/* Unlock the to_fill index saved in the shared_memory access
 */
void unlock_to_fill_sem();

/* Unlock the current cell of the masterbook shm in which the node had written the block
 * @param i_cell_block_list index to be blocked
 */
void unlock_masterbook_cell_access(int i_cell_block_list);

/* Advice the master porc via master_message queue by sending a master_message
 * @param termination_type MSG_REPORT_TYPE type process termination occured
 */
void advice_master_of_termination(long termination_type);

/*Extract randomly the time needed for processing from node_configuration
 *@return an integer that rappresent the time for processing
 * */
long int get_time_processing(void);

/* SysVar */
int shm_tp_id = -1;               /* Id of node_tp_shm*/
int semaphore_start_id = -1;      /*Id of the start semaphore arrays for sinc*/
int semaphore_masterbook_id = -1; /*Id of the masterbook semaphore for accessing the block matrix*/
int semaphore_to_fill_id = -1;    /* Id of the masterbook to_fill access semaphore*/
int queue_node_id = -1;           /* Identifier of the node queue id */
int queue_user_id = -1;           /* Identifier of the user queue id*/
int queue_master_id = -1;         /* Identifier of the master queue id*/
int node_end = 0;                 /* For values different from 0 the node proc must end*/
float current_block_reward = 0;   /* The current value of all node block reward*/
int last_signal;                  /* Last signal received*/
int friends = -1;                 /* Number of friends of the node*/
struct node current_node;                       /* Current representation of the node*/
struct conf node_configuration;                 /* Configuration File representation*/
struct shm_conf *shm_conf_pointer_node;         /* Ref to the shm for configuration of the node*/
struct shm_book_master *shm_masterbook_pointer; /* Ref to the shm for the masterbook shm */

int main(int argc, char const *argv[])
{
    DEBUG_MESSAGE("NODE PROCESS STARTED");
    struct sigaction sa;
    sigset_t sigmask;
    int failure_shm = 0;
    DEBUG_MESSAGE("NODE STATE SET TO INIT");
    current_node.exec_state = PROC_STATE_INIT;
    current_node.pid = getpid();
    /************************************
     *      CONFIGURATION FASE          *
     * **********************************/
    read_conf_node(&node_configuration);
    node_create(&current_node, getpid(), -1, 0, node_configuration.so_tp_size, SO_BLOCK_SIZE,
                node_configuration.so_reward, (Reward)&calc_reward);
    if (check_arguments(argc, argv) && set_signal_handler_node(sa, sigmask))
    {
        struct node_msg msg_rep;
        int is_unsed_node = 0;
        /*-----------------------*/
        /*  CONNECTING TO QUEUES */
        /*-----------------------*/
        connect_to_queues();
        /*-------------------------*/
        /*  SHARED MEM  CONFIG     */
        /*-------------------------*/
        attach_to_shms();
        /************************************
         *      SINC AND WAITING FASE       *
         * **********************************/
        /*---------------------------*/
        /*  SEMAPHORES ACQUISITION   */
        /*---------------------------*/
        acquire_semaphore_ids();

        /*---------------------------*/
        /*  SEMAPHORES SINC..        */
        /*---------------------------*/
        DEBUG_MESSAGE("NODE READY, ON START_SEM");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0)
        {
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO WAIT FOR START");
        }
        current_node.exec_state = PROC_STATE_RUNNING;
        DEBUG_MESSAGE("NODE PROCESS RUNNING");

        /****************************************
         *   PROCESSING OF TRANSACTION FASE     *
         * **************************************/
        while (node_end != 1 && failure_shm < MAX_FAILURE_SHM_LOADING)
        {
            process_node_transaction(&msg_rep);
            process_simple_transaction_type(&msg_rep);
            if (msg_rep.sender_pid == -1)
            { /**default value of sender_pid is -1, if it is -1 it means that the transaction is not valid*/
                is_unsed_node++;
            }
            if (get_num_transactions(current_node.transactions_pool) >= SO_BLOCK_SIZE &&
                process_node_block() == FALSE)
            {
                failure_shm++;
            }
        }
        if (failure_shm > MAX_FAILURE_SHM_LOADING)
        {
            advice_master_of_termination(UNUSED_PROC);
            ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO READ DATA FROM NODE_TP_SHM");
        }
        if (is_unsed_node >= MAX_UNSED_CICLE_OF_NODE_PROC)
        {
            advice_master_of_termination(UNUSED_PROC);
            ERROR_EXIT_SEQUENCE_NODE("UNUSED NODE TP PROC");
        }
        sleep(100);
    }
    advice_master_of_termination(TERMINATION_END_CORRECTLY);
    EXIT_PROCEDURE_NODE(0);
}

Bool read_conf_node()
{
    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION...");
    switch (load_configuration(&node_configuration))
    {
    case 0:
        break;
    case -1:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: MISSING FILE OR EMPTY");
    case -2:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: BROKEN SIMULTATION LOGIC, CHECK CONF. VALUE");
    case -3:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: NOT ENOUGH USERS FOR NODES");
    case -4:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: MIN MAX EXECUTION TIME WRONG");
    case -5:
        ERROR_EXIT_SEQUENCE_NODE(" DURING CONF. LOADING: NODE REWARD IS OVER POSSIBILITIES OF USERS");
    default:
        return FALSE;
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CONFIGURATION LOADED");
    return TRUE;
}

Bool check_arguments(int argc, char const *argv[])
{
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING ARGC AND ARGV...");
    if (argc < 2)
    {
        ERROR_EXIT_SEQUENCE_NODE("MISSING ARGUMENT");
    }
    current_node.node_id = atoi(argv[1]);
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING ARGC AND ARGV DONE");
    return TRUE;
}

Bool set_signal_handler_node(struct sigaction sa, sigset_t sigmask)
{
    DEBUG_NOTIFY_ACTIVITY_RUNNING("SETTING SIGNAL HANDLER...");
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signals_handler;
    sa.sa_mask = sigmask;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0 ||
        sigaction(SIGUSR1, &sa, NULL) < 0 ||
        sigaction(SIGUSR2, &sa, NULL) < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("ERROR DURING THE CREATION OF THE SIG HANDLER ");
    }
    return TRUE;
}

void signals_handler(int signum)
{
    DEBUG_SIGNAL("SIGNAL RECEIVED ", signum);
    last_signal = signum;
    struct master_msg_report master_msg;
    struct node_msg node_msg;
    struct Transaction * t;
    switch (signum)
    {
        case SIGINT:
            alarm(0); /*pending alarm removed*/
            current_node.exec_state = PROC_STATE_TERMINATED;
            advice_master_of_termination(SIGNALS_OF_TERM_RECEIVED);
            EXIT_PROCEDURE_NODE(0);
            break;
        case SIGALRM:
            alarm (1);
            /**
             * select a transaction from the pool and send it to a friend node into the message queue
             */
            int node_id = rand_int_n_pos(friends);
            *t = queue_head(current_node.transactions_pool);
            node_msg_snd(queue_node_id, &node_msg, MSG_NODE_ORIGIN_TYPE, t, current_node.pid, TRUE, node_configuration.so_retry, shm_conf_pointer_node->nodes_snapshots[node_id][2]);
            printf("NODE %d SENT TRANSACTION %f TO NODE %d\n", current_node.node_id, t->amount, node_id);
            queue_remove_head(current_node.transactions_pool);
            break;
        case SIGUSR1:
            /**
             * Receive new friends from master and update the friends list
             */
            node_msg_receive(queue_node_id,&node_msg, MSG_MASTER_ORIGIN_ID);
            friends |= node_msg.sender_pid;
            break;
        case SIGUSR2:
            master_msg_send(queue_master_id, &master_msg, INFO_BUDGET, NODE, current_node.pid, current_node.exec_state, TRUE, current_node.budget);
#ifdef DEBUG_NODE
            DEBUG_NOTIFY_ACTIVITY_DONE("{DEBUG_NODE}:= REPLIED TO MASTER DONE");
#endif
            break;
        default:
            break;
    }
}

void free_sysVar_node()
{
    int semaphore_start_value;
    /* If the process is in INIT STATE his termination can block the whole simulation. So it need to
     * lock the sart semaphore.
     * If the semaphore_start_id is impossible to retrive ( = -1) nothing can be done.
     * */
    if (current_node.exec_state == PROC_STATE_INIT && semaphore_start_id >= 0)
    {
        semaphore_start_value = semctl(semaphore_start_id, 0, GETVAL);
        if (semaphore_start_value < 0)
            ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE INFORMATION ON STARTING SEMAPHORE");
        else if (semaphore_start_value > 0 && (semaphore_lock(semaphore_start_id, 0) < 0))
        {
            ERROR_MESSAGE("IMPOSSIBLE TO EXECUTE THE FREE SYS VAR (prob. sem_lock not set so cannot be closed)");
        }
    }
    current_node.exec_state = PROC_STATE_TERMINATED;
}

void free_mem_node()
{
    free_node(&current_node);
}

void attach_to_shms(void)
{
    DEBUG_NOTIFY_ACTIVITY_RUNNING("ATTACHING TO SHM...");
    int shm_conf_id = -1; /* id to the shm_conf*/
    shm_conf_id = shmget(SHM_CONFIGURATION, sizeof(struct shm_conf), 0600);
    if (shm_conf_id < 0)
    {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO ACCESS SHM CONF");
    }
    shm_conf_pointer_node = shmat(shm_conf_id, NULL, 0);
    if (shm_conf_pointer_node == (void *)-1)
    {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO SHM CONF");
    }
    friends = shm_conf_pointer_node->nodes_snapshots[get_node_position_by_pid(shm_conf_pointer_node,current_node.pid)];
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO SHM DONE");
    shm_conf_id = shmget(MASTER_BOOK_SHM_KEY, sizeof(struct shm_book_master), 0600);
    if (shm_conf_id < 0)
    {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO ACCESS SHM BOOKMASTER");
    }
    shm_masterbook_pointer = shmat(shm_conf_id, NULL, 0);
    if (shm_masterbook_pointer == (void *)-1)
    {
        advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO THE SHM_MASTERBOOK");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("ATTACHING TO SHM DONE");
}

void connect_to_queues(void)
{
    queue_node_id = msgget(NODES_QUEUE_KEY, 0600);
    if (queue_node_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO NODE MESSAGE QUEUE");
    }
    queue_user_id = msgget(USERS_QUEUE_KEY, 0600);
    if (queue_user_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO USER QUEUE");
    }
    queue_master_id = msgget(MASTER_QUEUE_KEY, 0600);
    if (queue_master_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO CONNECT TO MASTER QUEUE");
    }
}

int process_node_block()
{
    /*Loading them into the node_block_transactions*/
    if (load_block() == TRUE &&
        get_num_transactions(current_node.transactions_block) == SO_BLOCK_SIZE /* DID i got the correct num of transactions*/ &&
        current_node.calc_reward(&current_node, current_node.percentage, TRUE, &current_block_reward) >= 0)
    {
        int num_of_shm_retry = 0;
        while (num_of_shm_retry < MAX_FAILURE_SHM_BOOKMASTER_LOCKING && lock_shm_masterbook() == FALSE)
        {
            num_of_shm_retry++;
        }
        /*send confirmed to all users*/
        adv_users_of_block();
    }

    return 0;
}
Bool lock_shm_masterbook(void)
{
    DEBUG_NOTIFY_ACTIVITY_RUNNING("NODE:= LOCKING THE SHM FOR ADDING THE BLOCK...");
    struct timespec trans_proc_sim;
    trans_proc_sim.tv_sec = 0;
    trans_proc_sim.tv_nsec = get_time_processing();
    nanosleep(&trans_proc_sim, (void *)NULL);
    lock_to_fill_sem();
    unsigned short int i_cell_block_list = shm_masterbook_pointer->to_fill;
    /*Inserting the block into the shm*/
    shm_masterbook_pointer->to_fill += 1;
    lock_masterbook_cell_access(i_cell_block_list);
    struct Transaction block_list[get_num_transactions(current_node.transactions_block)];
    queue_to_array(current_node.transactions_block, block_list);
    if (insert_block(shm_masterbook_pointer, block_list) == 0)
    {
        current_node.budget = current_block_reward;
    }
    else
    {
        ERROR_MESSAGE("IMPOSSIBLE TO INSERT BLOCK");
        return FALSE;
    }
    /*Unloacking the semaphore*/
    unlock_masterbook_cell_access(i_cell_block_list);
    unlock_to_fill_sem();
    DEBUG_NOTIFY_ACTIVITY_DONE("NODE:= LOCKING THE SHM FOR ADDING THE BLOCK DONE");
    return TRUE;
}

void lock_to_fill_sem(void)
{
    while (semctl(semaphore_to_fill_id, 0, GETVAL) < 0 && semaphore_lock(semaphore_to_fill_id, 0) < 0)
    {
        if (errno == EINTR || semctl(semaphore_to_fill_id, 0, GETVAL) < 0)
        {
            if (last_signal == SIGALRM)
            {
                /* RICEZIONE DI SEGNALE*/
                unlock_to_fill_sem();
                /*Avvisare il main e il processo deve terminare*/
                advice_master_of_termination(IMPOSSIBLE_TO_SEND_TRANSACTION);
            }
            else
            {
                continue;
            }
        }
        else
        {
            advice_master_of_termination(IMPOSSIBLE_TO_CONNECT_TO_SHM);
            ERROR_EXIT_SEQUENCE_NODE("ERROR WHILE TRYING TO EXEC LOCK ON TO_FILL ACCESS SEM");
        }
    }
}

void lock_masterbook_cell_access(int i_cell_block_list)
{
    while (semctl(semaphore_masterbook_id, i_cell_block_list, GETVAL) < 0 && semaphore_lock(semaphore_masterbook_id, i_cell_block_list) < 0)
    {
        /*TODO: fare refactoring nei due while*/
        if (errno == EINTR || semctl(semaphore_masterbook_id, i_cell_block_list, GETVAL) < 0)
        {
            if (last_signal == SIGALRM)
            {
                /**RICEZIONE DI SEGNALE*/
                unlock_masterbook_cell_access(i_cell_block_list);
                /*Avvisare il main e il processo deve terminare*/
                advice_master_of_termination(IMPOSSIBLE_TO_SEND_TRANSACTION);
            }
            else
            {
                continue;
            }
        }
        else
        {
            ERROR_EXIT_SEQUENCE_NODE("ERROR WHILE TRYING TO EXEC LOCK ON TO_FILL ACCESS SEM");
        }
    }
}

void unlock_to_fill_sem(void)
{
    while (semaphore_unlock(semaphore_to_fill_id, 0) < 0)
    {
        if (errno != EINTR)
        {
            ERROR_EXIT_SEQUENCE_NODE("ERROR DURING THE UNLOCK OF THE SEMAPHORE");
        }
    }
}

void unlock_masterbook_cell_access(int i_cell_block_list)
{
    while (semaphore_unlock(semaphore_masterbook_id, i_cell_block_list) < 0)
    {
        if (errno != EINTR)
        {
            ERROR_EXIT_SEQUENCE_NODE("ERROR DURING THE UNLOCK OF BOOKMASTER CELL");
        }
    }
}

Bool load_block(void)
{
    if (queue_is_empty(current_node.transactions_block) == FALSE || get_num_transactions(current_node.transactions_pool) < SO_BLOCK_SIZE)
    {
        return FALSE;
    }
    /*Loading the transactions from the pool to the block*/
    if (queue_copy_n_transactions(current_node.transactions_pool, current_node.transactions_block, SO_BLOCK_SIZE) == TRUE)
    {
        return TRUE;
    }
    else
        return FALSE;
}

void advice_master_of_termination(long termination_type)
{
    struct master_msg_report termination_report;
    current_node.exec_state = PROC_STATE_TERMINATED;
    if (master_msg_send(queue_master_id, &termination_report, termination_type, NODE, current_node.pid, current_node.exec_state, TRUE, current_node.budget) < 0)
    {
        char *error_string = strcat("IMPOSSIBLE TO ADVICE MASTER OF : %s", from_type_to_string(termination_type));
        ERROR_MESSAGE(error_string);
    }
}

void acquire_semaphore_ids(void)
{
    semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, 0);
    if (semaphore_start_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN ID OF START SEM");
    }
    if (semaphore_lock(semaphore_start_id, 0) < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN THE START SEMAPHORE");
    }
    semaphore_masterbook_id = semget(SEMAPHORE_MASTER_BOOK_ACCESS_KEY, 1, 0);
    if (semaphore_masterbook_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN THE MASTERBOOK SEM");
    }
    semaphore_to_fill_id = semget(SEMAPHORE_MASTER_BOOK_TO_FILL_KEY, 1, 0);
    if (semaphore_to_fill_id < 0)
    {
        ERROR_EXIT_SEQUENCE_NODE("IMPOSSIBLE TO OBTAIN MASTERBOOK_TO_FILL SEM");
    }
}

long int get_time_processing(void)
{
    srand(getpid());
    return (rand() %
            (node_configuration.so_max_trans_gen_nsec - node_configuration.so_min_trans_proc_nsec + 1)) +
           node_configuration.so_min_trans_proc_nsec;
}

void adv_users_of_block(void)
{
    int sender_pid = -1;
    int receiver_pid = -1;
    while (queue_is_empty(current_node.transactions_block) == FALSE)
    {
        struct user_msg *u_msg_rep = (struct user_msg *)malloc(sizeof(struct user_msg));
        struct Transaction t = queue_head(current_node.transactions_block);
        t.t_type = TRANSACTION_SUCCES;
        sender_pid = t.sender;
        receiver_pid = t.reciver;
        int queue_id_user_proc = get_queueid_by_pid(shm_conf_pointer_node, sender_pid, TRUE);
        if (queue_id_user_proc < 0)
        {
            ERROR_MESSAGE("ILLIGAL PID INTO TRANSACTION, NO PIDS FOUND");
            return;
        }
        user_msg_snd(queue_user_id, u_msg_rep, MSG_TRANSACTION_CONFIRMED_TYPE, &t, current_node.pid, TRUE, queue_id_user_proc);
        queue_id_user_proc = get_queueid_by_pid(shm_conf_pointer_node, receiver_pid, TRUE);
        if (queue_id_user_proc < 0)
        {
            ERROR_MESSAGE("ILLIGAL PID INTO TRANSACTION, NO PIDS FOUND");
            return;
        }
        user_msg_snd(queue_user_id, u_msg_rep, MSG_TRANSACTION_INCOME_TYPE, &t, current_node.pid, TRUE, queue_id_user_proc);
        queue_remove_head(current_node.transactions_block);
    }
    current_node.budget += current_block_reward;
}
void process_simple_transaction_type(struct node_msg *msg_rep)
{
    if (node_msg_receive(queue_node_id, msg_rep, current_node.node_id) == 0)
    {
        DEBUG_MESSAGE("NODE TRANSACTION TYPE RECEIVED");
        if (get_num_transactions(current_node.transactions_pool) < current_node.tp_size){
            queue_append(current_node.transactions_pool, msg_rep->t);
        }
        else if (msg_rep->t.hops < node_configuration.so_hops){
            /**TP_SIZE FULL
             * Sending the transaction to a friend
             * */
            printf("TP_SIZE FULL\n");
            node_msg_snd(queue_node_id, msg_rep, MSG_NODE_ORIGIN_TYPE, &msg_rep->t, current_node.node_id, TRUE,node_configuration.so_retry,shm_conf_pointer_node->nodes_snapshots[get_rand_one(friends)][2]);
        }else{
            /*TP_SIZE FULL AND HOPS EXCEEDED
             * Reporting the transaction to the master
             * */
            printf("TP_SIZE FULL AND HOPS EXCEEDED\n");
            struct master_msg_report master_msg;
            /**TODO: implement master msg_send*/
        }
    }
    else
    {
        msg_rep->sender_pid = -1;
    }
}

void process_node_transaction(struct node_msg *msg_rep)
{
    /*TODO: Implement incoming transaction from other node*/
    if (node_msg_receive(queue_node_id, msg_rep, current_node.node_id - MSG_NODE_ORIGIN_TYPE) == 0)
    {
        DEBUG_MESSAGE("NODE TRANSACTION RECEIVED");
        queue_append(current_node.transactions_pool, msg_rep->t);
    }
    else
    {
        msg_rep->sender_pid = -1;
    }
}
