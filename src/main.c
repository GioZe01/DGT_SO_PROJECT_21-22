#define _GNU_SOURCE
/* Std  */
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

/*  LOCAL IMPORT*/
#include "local_lib/headers/glob.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/simulation_errors.h"
#include "local_lib/headers/user_msg_report.h"
#include "local_lib/headers/master_msg_report.h"
#include "local_lib/headers/node_msg_report.h"
#include "local_lib/headers/process_info_list.h"
#include "local_lib/headers/conf_shm.h"
#include "local_lib/headers/book_master_shm.h"

#ifdef DEBUG
#ifdef DEBUG_MAIN
#include "local_lib/headers/debug_utility.h"
#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#define DEBUG_BLOCK_ACTION_START(mex)
#define DEBUG_BLOCK_ACTION_END()
#endif
#endif


/* Support functions*/
/*  ! All the following functions are capable of EXIT_PROCEDURE*/
/**
 * Check for messages in the master msg_queue
 * @param msg_report pointer to the struct representing the msg in the master msg queue
 * @return -1 in case of FAILURE. 0 otherwise
 */
int check_msg_report(struct master_msg_report *msg_report);

/**
 * Create the shared memory for configuration purposes of kid processes
 */
void create_shm_conf(void);

/**
 * Create the shared memory for the master-book
 */
void create_masterbook(void);

/**
 * Create the message queue for kid to master communication
 */
void create_master_msg_report_queue(void);

/**
 * Create the semaphores needed for the access regulation for this specific program
 */
void create_semaphores(void);

/**
 * \brief Create the users processes and lunch them with execve
 * utilize simulation configuration in the file conf for generating the exact number of users
 * @param users_pids pointer to an array of pids to be filled with the generated once
 * @param users_queues_ids pointer to an array of pids to be filled with the generated once
 * @return -1 in case of FAILURE. 0 otherwise
 */
int create_users_proc(int *users_pids, int *users_queues_ids);

/**
 * Create the users message queues for processes to users communication
 */
void create_users_msg_queue(void);

/**
 * Create the nodes message queues for processes to nodes communication
 */
void create_nodes_msg_queue(void);

/**
 * \brief Create the nodes processes and lunch the with execve
 * utilize simulation configuration in the file conf for generating the exact number of nodes
 * @param nodes_pids pointer to an array of pids to be filled with the generated once
 * @param nodes_queues_ids pointer to an array of pids to be filled with the generate once
 * @return -1 in case of FAILURE. 0 otherwise
 */
int create_nodes_proc(int *nodes_pids, int *nodes_queues_ids);

/**
 * Print all the information requested by the project specification
 */
void print_info(void);

/**
 * \brief Read the conf file present in te project dir
 * load the configuration directly in the struct conf configuration that is a SysVar
 */
Bool read_conf(void);

/**
 * Set the handler for signals of the current main_proc
 * @param sa  describe the type of action to performed when a signal arrive
 */
void set_signal_handlers(struct sigaction sa);

/**
 * handler of the signal
 * @param signum type of signal to be handled
 */
void signals_handler(int signum);

/*
 * Update the budget of kids process saved into the proc_list via master message queue
 */
void update_kids_info(void);

/* Variables*/
struct conf simulation_conf; /*Structure representing the configuration present in the conf file*/
ProcList proc_list; /* Pointer to a linked list of all proc generated*/
struct shm_conf *shm_conf_pointer; /* Pointer to the shm_conf structure in shared memory*/
struct shm_book_master *shm_masterbook_pointer; /* Pointer to the shm_masterbook structure in shared memory*/
int simulation_end = 0; /* For value different from 0 the simulation must end*/
int shm_conf_id = -1; /* Id of the shm for the configuration of the node*/
int shm_masterbook_id = -1; /* Id of the shm for the master book */
int msg_report_id_master = -1;/* Identifier for message queue for master communication*/
int msg_report_id_users = -1; /* Identifier for message queue for users communication*/
int msg_report_id_nodes = -1; /* Identifier for message queue for nodes communication*/
int semaphore_start_id = -1;  /* Id of the start semaphore arrays for sinc*/
int semaphore_masterbook_id = -1; /* Id of the masterbook semaphore for accessing the block matrix */
int semaphore_to_fill_id = -1; /* Id of the masterbook to_fill access semaphore*/
pid_t main_pid; /*pid of the current proc*/

int main() {
    /* semctl(15, 0, IPC_RMID); TODO: Remove*/
    main_pid = getpid();
    if (read_conf() == TRUE) {
        /*  Local Var Declaration   */
        struct sigaction sa; /*Structure for handling signals */
        struct master_msg_report msg_repo;
        int *users_pids; /*in 0 position is saved the actual size of the array of pids saved in the pointer*/
        int *nodes_pids; /*in 0 position is saved the actual size of the array of pids saved in the pointer*/
        int *users_queues_ids;/*in 0 position is saved the actual size of the array of ids saved in the pointer*/
        int *nodes_queues_ids;/*in 0 position is saved the actual size of the array of ids saved in the pointer*/
        /* Pointers allocation  */
        users_pids = (int *) malloc(sizeof(int) * simulation_conf.so_user_num);
        nodes_pids = (int *) malloc(sizeof(int) * simulation_conf.so_nodes_num);
        users_queues_ids = (int *) malloc(sizeof(int) * simulation_conf.so_user_num);
        nodes_queues_ids = (int *) malloc(sizeof(int) * simulation_conf.so_nodes_num);
        /************************************
         *      CONFIGURATION FASE
         * ***********************************/
        proc_list = proc_list_create();
        set_signal_handlers(sa);
        create_semaphores();
        create_masterbook();
        create_shm_conf();
        /*************************************
         *  CREATION OF CHILD PROCESSES FASE *
         * ***********************************/

        /*-------------------------------*/
        /*  CREATING THE QUEUES          *
        /*-------------------------------*/
        create_users_msg_queue();
        create_nodes_msg_queue();
        create_master_msg_report_queue();
        /*-------------------------*/
        /*  CREATION OF PROCESSES  *
        /*-------------------------*/

        DEBUG_BLOCK_ACTION_START("PROC GENERATION");
        /* Crating users*/
        if (create_users_proc(users_pids, users_queues_ids) < 0) { ERROR_MESSAGE("IMPOSSIBLE TO CREATE USERS PROC"); }
        /* Creating nodes*/
        if (create_nodes_proc(nodes_pids, nodes_queues_ids) < 0) { ERROR_MESSAGE("IMPOSSIBLE TO CREATE NODES PROC"); }
        DEBUG_BLOCK_ACTION_END();

        DEBUG_NOTIFY_ACTIVITY_RUNNING("SHM INITIALIZING...");
        if (shm_conf_create(shm_conf_pointer, users_pids, users_queues_ids, nodes_pids, nodes_queues_ids) < 0) {
            ERROR_EXIT_SEQUENCE_MAIN("FAILED ON SHM_CONF INITIALIZING");
        };
        free(nodes_queues_ids);
        free(nodes_pids);
        free(users_pids);
        free(users_queues_ids);
        DEBUG_NOTIFY_ACTIVITY_DONE("SHM INITIALIZING DONE");

#ifdef DEBUG
        shm_conf_print(shm_conf_pointer);
#endif
        DEBUG_BLOCK_ACTION_START("WAITING CHILDREN");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO WAIT ON SEM_START");
        }
        DEBUG_MESSAGE("WAITING DONE");
        DEBUG_BLOCK_ACTION_END();
        alarm(1);
        while (simulation_end != 1) {
            if (check_msg_report(&msg_repo) < 0) {
                /*If a message arrive make the knowledge*/
                ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO RETRIEVE INFO FROM MSG_QUEUE");
            }
        }
        printf("====TIME FINISHED====\n");
        kill_kids();
        /*TODO: verifica messaggi di report*/
        wait_kids();
        /*TODO: final printing*/
    }
    free_mem();
    free_sysVar();
    exit(0);
}

/**
 *  Create a new user proc
 * @return -1 if fail. 0 otherwise
 */
int create_users_proc(int *users_pids, int *users_queues_ids) {
    char *argv_user[] = {PATH_TO_USER, NULL, NULL}; /*Future addon*/
    pid_t user_pid;
    int i, queue_id = DELTA_USER_MSG_TYPE;
    users_pids[0] = 0;
    users_queues_ids[0] = 0;
    for (i = 0; i < simulation_conf.so_user_num; i++) {
        argv_user[1] = (char *) malloc(11 * sizeof(char));
        switch (user_pid = fork()) {
            case -1:
                if (argv_user[1] != NULL) free(argv_user[1]);
                if (argv_user[2] != NULL) free(argv_user[2]);
                return -1;
            case 0: /*kid*/
                sprintf(argv_user[1], "%d", queue_id);/*Let the user know is position*/
                execve(argv_user[0], argv_user, NULL);
                ERROR_MESSAGE("IMPOSSIBLE TO CREATE A USER");
                return -1;
            default: /*father*/
                users_queues_ids[i + 1] = queue_id;
                users_pids[i + 1] = user_pid;
                users_pids[0] += 1;
                users_queues_ids[0] += 1;
                insert_in_list(proc_list, user_pid, PROC_TYPE_USER, queue_id);
                /*Free if utilized pointers to argv*/
                if (argv_user[1] != NULL) free(argv_user[1]);
                if (argv_user[2] != NULL) free(argv_user[2]);
                queue_id += DELTA_USER_MSG_TYPE;
                DEBUG_MESSAGE("USER CREATED");
                break;
        }
    }
    return 0;
}

/**
 * Create a new node proc
 * @return -1 if fail. 0 otherwise
 */
int create_nodes_proc(int *nodes_pids, int *nodes_queues_ids) {
    char *argv_node[] = {PATH_TO_NODE, NULL, NULL}; /*Future addon*/
    pid_t node_pid;
    int i, queue_id = DELTA_NODE_MSG_TYPE;
    nodes_pids[0] = 0;
    nodes_queues_ids[0] = 0;
    for (i = 0; i < simulation_conf.so_nodes_num; i++) {
        argv_node[1] = (char *) malloc(11 * sizeof(char));
        switch (node_pid = fork()) {
            case -1:
                if (argv_node[1] != NULL) free(argv_node[1]);
                if (argv_node[2] != NULL) free(argv_node[2]);
                return -1;
            case 0: /*kid*/
                sprintf(argv_node[1], "%d", queue_id);/*Let the user know is position*/
                execve(argv_node[0], argv_node, NULL);
                ERROR_MESSAGE("IMPOSSIBLE TO CREATE A NODE");
                return -1;
            default: /*father*/
                nodes_queues_ids[i + 1] = queue_id;
                nodes_pids[i + 1] = node_pid;
                nodes_pids[0] += 1;
                nodes_queues_ids[0] += 1;
                insert_in_list(proc_list, node_pid, PROC_TYPE_NODE, queue_id);
                /*Free if utilized pointers to argv*/
                if (argv_node[1] != NULL) free(argv_node[1]);
                if (argv_node[2] != NULL) free(argv_node[2]);
                queue_id += DELTA_NODE_MSG_TYPE;
                DEBUG_MESSAGE("NODE CREATED");
                break;
        }
    }
    return 0;
}


void set_signal_handlers(struct sigaction sa) {
    DEBUG_BLOCK_ACTION_START("SIGNAL HANDLERS");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("SETTING SIGNALS HANDLERS...");
    memset(&sa, 0, sizeof(sa));/*initialize the structure*/
    sa.sa_handler = signals_handler;
    if (sigaction(SIGTSTP, &sa, NULL) < 0 ||
        sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0) {
        ERROR_MESSAGE("ERROR SETTING SIGNAL HANDLERS");
        EXIT_PROCEDURE_MAIN(EXIT_FAILURE);
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("SETTING HANDLERS DONE");
    DEBUG_BLOCK_ACTION_END();
}

/**
 * Handler for the signals SIGINT, SIGTERM, SIGALARM
 * @param signum
 */
void signals_handler(int signum) { /*TODO: Scrivere implementazione*/
    int old_errno;
    static int num_inv = 0;

    old_errno = errno;
    DEBUG_SIGNAL("SIGNAL RECEIVED", signum);
    switch (signum) {
        case SIGINT:
        case SIGTERM:
            if (getpid() == main_pid) { EXIT_PROCEDURE_MAIN(0); }
            else { exit(0); }
        case SIGALRM:
            if (getpid() == main_pid) {
                num_inv++;
                /*request info from kids */
                update_kids_info();
                /*Printing infos*/
                if (num_inv == simulation_conf.so_sim_sec) simulation_end = 1;
                else alarm(1);
                /*TODO: METTO IN PAUSA I NODI vedere se mettere anche in pausa i processi user*/
            }
            break;
        default:
            break;
    }
    errno = old_errno;
}

void create_semaphores(void) {
    DEBUG_BLOCK_ACTION_START("CREATE SEM");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF START_SEMAPHORE CHILDREN....");
    semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semaphore_start_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE START_SEMAPHORE");
    }

    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF START_SEMAPHORE CHILDREN DONE");

    DEBUG_NOTIFY_ACTIVITY_RUNNING("INITIALIZATION OF START_SEMAPHORE CHILDREN....");
    if (semctl(semaphore_start_id, 0, SETVAL, simulation_conf.so_user_num + (2*simulation_conf.so_nodes_num))<
            0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO INITIALISE SEMAPHORE START CHILDREN");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("INITIALIZATION OF START_SEMAPHORE CHILDREN DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF MASTEBOOK ACCESS SEM....");

    semaphore_masterbook_id = semget(SEMAPHORE_MASTER_BOOK_ACCESS_KEY, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semaphore_masterbook_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE MASTERBOOK SEM");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF MASTEBOOK ACCESS SEM...");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("INITIALIZATION OF MASTERBOOK ACCESS SEM....");
    if (semctl(semaphore_masterbook_id, 0, SETVAL, SO_REGISTRY_SIZE) <
            0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO INITIALISE SEMAPHORE START CHILDREN");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("INITIALIZATION OF MASTEBOOK ACCESS SEM....");
    DEBUG_BLOCK_ACTION_END();

    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF TO_FILL SEM....");
    semaphore_to_fill_id = semget(SEMAPHORE_MASTER_BOOK_TO_FILL_KEY, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semaphore_start_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE TO_FILL SEM");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF TO_FILL DONE");
}

void wait_kids() {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("WAITING KIDS...");
    saving_private_ryan(proc_list);
    DEBUG_NOTIFY_ACTIVITY_DONE("WAITING KIDS DONE");
}

void kill_kids() {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("KILLING KIDS...");
    terminator(proc_list);
    DEBUG_NOTIFY_ACTIVITY_DONE("KILLING KIDS DONE");
}

void free_mem() {
    list_free(proc_list);
}

void free_sysVar() {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING STARTING SEMAPHORE...");
    if (semaphore_start_id >= 0 && semctl(semaphore_start_id, 0, IPC_RMID) < 0)
        ERROR_MESSAGE("REMOVING PROCEDURE FOR START_SEM HAS FAILED");
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING STARTING SEMAPHORE DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING MASTERBOOK SEMAPHORE...");
    if (semaphore_masterbook_id>= 0 && semctl(semaphore_masterbook_id, 0, IPC_RMID) < 0)
        ERROR_MESSAGE("REMOVING PROCEDURE FOR MASTERBOOK SEM HAS FAILED");
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING MASTERBOOK SEMAPHORE DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING TO_FILL MASTERBOOK SEMAPHORE...");
    if (semaphore_to_fill_id>= 0 && semctl(semaphore_to_fill_id, 0, IPC_RMID) < 0)
        ERROR_MESSAGE("REMOVING PROCEDURE FOR TO_FILL MASTERBOOK SEM HAS FAILED");
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING TO_FILL MASTERBOOK SEMAPHORE DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING SHM CONF...");
    if (shm_conf_id >= 0 && shmctl(shm_conf_id, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("REMOVING PROCEDURE FOR SHM_CONF FAILED");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING SHM CONF DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING SHM MASTERBOOK...");
    if (shm_masterbook_id>= 0 && shmctl(shm_masterbook_id, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("REMOVING PROCEDURE FOR SHM_MASTERBOOK FAILED");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING SHM MASTERBOOK DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING MASTER QUEUE...");
    if (msg_report_id_master >= 0 && msgctl(msg_report_id_master, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO DELETE MESSAGE QUEUE OF MASTER");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING MASTER QUEUE DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING USER QUEUE...");
    if (msg_report_id_users >= 0 && msgctl(msg_report_id_users, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO DELETE MESSAGE QUEUE OF USER");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING USER QUEUE DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING NODE QUEUE...");
    if (msg_report_id_nodes >= 0 && msgctl(msg_report_id_nodes, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO DELETE MESSAGE QUEUE OF NODE");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING NODE QUEUE DONE");
}

Bool read_conf(void) {
    DEBUG_BLOCK_ACTION_START("READING CONF");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("READING CONFIGURATION...");
    switch (load_configuration(&simulation_conf)) {
        case 0:
            break;
        case -1:
        ERROR_EXIT_SEQUENCE_MAIN(" DURING CONF. LOADING: MISSING FILE OR EMPTY");
        case -2:
        ERROR_EXIT_SEQUENCE_MAIN(" DURING CONF. LOADING: BROKEN SIMULATION LOGIC, CHECK CONF. VALUE");
        case -3:
        ERROR_EXIT_SEQUENCE_MAIN(" DURING CONF. LOADING: NOT ENOUGH USERS FOR NODES");
        case -4:
        ERROR_EXIT_SEQUENCE_MAIN("DURING CONF. LOADING: MIN MAX EXECUTION TIME WRONG");
        case -5:
        ERROR_EXIT_SEQUENCE_MAIN(" DURING CONF. LOADING: NODE REWARD IS OVER POSSIBILITIES OF USERS");
        default:
            return FALSE;
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("READING CONFIGURATION DONE");
    DEBUG_BLOCK_ACTION_END();
    return TRUE;
}

void create_master_msg_report_queue(void) {
    DEBUG_BLOCK_ACTION_START("MSG REPORT QUEUE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING THE MESSAGE REPORT QUEUE...");
    msg_report_id_master = msgget(MASTER_QUEUE_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if (msg_report_id_master < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE MESSAGE REPORT QUEUE");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING THE MESSAGE REPORT QUEUE DONE");
    DEBUG_BLOCK_ACTION_END();

}

int check_msg_report(struct master_msg_report *msg_report) {
    struct msqid_ds msg_rep_info;
    if (msgctl(msg_report_id_master, IPC_STAT, &msg_rep_info) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE MESSAGE QUEUE INFO");
        return -1;
    } else {
        /*fetching all msg if present*/
        if (msg_rep_info.msg_qnum != 0 &&
            msgrcv(msg_report_id_master, msg_report, sizeof(*msg_report) - sizeof(long), 0, 0)) {
            /*make_knowledge*/
        }
        return 0;
    }
}

void create_users_msg_queue(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING MSG REPORT QUEUE FOR USERS....");
    msg_report_id_users = msgget(USERS_QUEUE_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if (msg_report_id_users < 0) { ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE USER MESSAGE QUEUE"); }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING MSG REPORT QUEUE FOR USERS DONE");
}

void create_nodes_msg_queue(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING MSG REPORT QUEUE FOR NODES...");
    msg_report_id_nodes = msgget(NODES_QUEUE_KEY, IPC_CREAT | IPC_CREAT | 0600);
    if (msg_report_id_nodes < 0) { ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE NODES MESSAGE QUEUE"); }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING MSG REPORT QUEUE FOR NODES DONE");
}

void create_shm_conf(void) {
    DEBUG_BLOCK_ACTION_START("SHM_CONF");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING SHM_CONF....");
    shm_conf_id = shmget(SHM_CONFIGURATION, sizeof(struct shm_conf), IPC_CREAT | IPC_EXCL | 0600);
    if (shm_conf_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE CONF SHARED MEMORY");
    }
    shm_conf_pointer = shmat(shm_conf_id, NULL, 0);
    if (shm_conf_pointer == (void *) -1) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CONNECT TO THE CONF SHARED MEMORY");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING SHM_CONF DONE");
    DEBUG_BLOCK_ACTION_END();
}

void create_masterbook() {
    DEBUG_BLOCK_ACTION_START("CREATE MASTERBOOK");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING THE MASTER_BOOK....");
    shm_masterbook_id = shmget(MASTER_BOOK_SHM_KEY, sizeof(struct shm_book_master), IPC_CREAT | IPC_EXCL | 0600);
    if (shm_masterbook_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE SHARED MEM FOR MASTERBOOK");
    }
    shm_masterbook_pointer = shmat(shm_masterbook_id, NULL, 0);
    if (shm_masterbook_id == (void *) -1) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CONNECT TO THE MASTERBOOK SHM");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING THE MASTER BOOK DOONE");
    DEBUG_BLOCK_ACTION_END();
}
void print_info(void){
    printf("====== INFO ======\n");

}
void update_kids_info(void){
    int num_msg_to_wait_for = send_sig_to_all(proc_list, SIGUSR2);
    if(num_msg_to_wait_for<0){
        ERROR_MESSAGE("IMPOSSIBLE TO UPDATE KIDS INFO");
    }
    else if (num_msg_to_wait_for== 0){
        DEBUG_MESSAGE("NO PROCESS TO UPDATE");
    }
    struct master_msg_report * msg_rep;
    do{
        alarm(MAX_WAITING_TIME_FOR_UPDATE);/*cannot loop forever*/
        master_msg_receive_info(msg_report_id_master, msg_rep);
        Proc proc_to_update = get_proc_from_pid(proc_list,msg_rep->sender_pid);
        proc_to_update->budget = msg_rep->budget;
        num_msg_to_wait_for--;
    }while(num_msg_to_wait_for!=0);
}
