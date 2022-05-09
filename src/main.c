/* Std  */
#include <stdio.h>
#include <stdlib.h>
/* Sys  */
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
/*  General */
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

/*  LOCAL IMPORT*/
#include "local_lib/headers/glob.h"
#include "local_lib/headers/int_condenser.h"
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
#include "local_lib/headers/debug_utility.h"

/* Support functions*/
/*  ! All the following functions are capable of EXIT_PROCEDURE*/


/**
 * @brief Check if masterbook is full or not. If full, return FALSE, else TRUE. Check if there're
 * enough process running to fill the masterbook.
 */
Bool check_runnability();

/**
 * Create the shared memory for configuration purposes of kid processes
 */
void create_shm_conf(void);

/**
 * Create a node_proc as support friend process in case of TP_FULL
 * @param node_id The queue_id of the node to be created
 * @return The pid of the created process
 */
int create_node_proc(int new_node_id);


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
 * @brief Create the users processes and lunch them with execve
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
 * @brief Create the nodes processes and lunch the with execve
 * utilize simulation configuration in the file conf for generating the exact number of nodes
 * @param nodes_pids pointer to an array of pids to be filled with the generated once
 * @param nodes_queues_ids pointer to an array of pids to be filled with the generate once
 * @return -1 in case of FAILURE. 0 otherwise
 */
int create_nodes_proc(int *nodes_pids, int *nodes_queues_ids);

/**
 * @brief Get the string related to the simulation end code
 */
char *get_end_simulation_msg();

/**
 * @brief Return an array of int with the friends of each nodes in the simulation
 * @param nodes_friends pointer to an array of int to be filled with the generated once
 * @param nodes pointer to an array of int with the nodes in the simulation (in 0 index there's
 * the number of nodes)
 */
void generate_nodes_friends_array(int *nodes_friends, int *nodes);

/**
 * @brief End the simulation by killing all the processes and cleaning the shared memory
 */
void end_simulation();

/**
 * Print all the information requested by the project specification
 */
void print_info(void);

/**
 * @brief Read the conf file present in te project dir
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

/**
 * Update the budget of kids process saved into the proc_list via master message queue
 */
void update_kids_info(void);

/**
 * @brief Lock the semaphore for to fill masterbook shared memory
 */
void lock_to_fill_sem(void);

/**
 * @brief unlock the semaphore to fill the master book
 */
void unlock_to_fill_sem(void);

/* Variables*/
struct conf simulation_conf;                    /**< Structure representing the configuration present in the conf file*/
ProcList proc_list;                             /**< Pointer to a linked list of all proc generated*/
struct shm_conf *shm_conf_pointer;              /* Pointer to the shm_conf structure in shared memory*/
struct shm_book_master *shm_masterbook_pointer; /* Pointer to the shm_masterbook structure in shared memory*/
int simulation_end = -1;                        /* For value different >= 0 the simulation must end*/
int shm_conf_id = -1;                           /* Id of the shm for the configuration of the node*/
int shm_masterbook_id = -1;                     /* Id of the shm for the master book */
int msg_report_id_master = -1;                  /* Identifier for message queue for master communication*/
int msg_report_id_users = -1;                   /* Identifier for message queue for users communication*/
int msg_report_id_nodes = -1;                   /* Identifier for message queue for nodes communication*/
int semaphore_start_id = -1;                    /* Id of the start semaphore arrays for sinc*/
int semaphore_masterbook_id = -1;               /* Id of the masterbook semaphore for accessing the block matrix */
int semaphore_to_fill_id = -1;                  /* Id of the masterbook to_fill access semaphore*/
int last_signal = -1;                           /* Last signal received*/
pid_t main_pid;                                 /*pid of the current proc*/
Bool printed = FALSE;

int main() {
    main_pid = getpid();
    if (read_conf() == TRUE) {
        /*  Local Var Declaration   */
        struct sigaction sa; /*Structure for handling signals */
        struct master_msg_report msg_repo;
        int *users_pids;       /*in 0 position is saved the actual size of the array of pids saved in the pointer*/
        int *nodes_pids;       /*in 0 position is saved the actual size of the array of pids saved in the pointer*/
        int *users_queues_ids; /*in 0 position is saved the actual size of the array of ids saved in the pointer*/
        int *nodes_queues_ids; /*in 0 position is saved the actual size of the array of ids saved in the pointer*/
        /* Pointers allocation  */
        users_pids = (int *) malloc(sizeof(int) * (simulation_conf.so_user_num + 1));
        nodes_pids = (int *) malloc(sizeof(int) * (simulation_conf.so_nodes_num + 1));
        users_queues_ids = (int *) malloc(sizeof(int) * (simulation_conf.so_user_num + 1));
        nodes_queues_ids = (int *) malloc(sizeof(int) * (simulation_conf.so_nodes_num + 1));
        /************************************
         *      CONFIGURATION FASE          *
         * ***********************************/
        proc_list = proc_list_create();
        set_signal_handlers(sa);
        create_semaphores();
        create_masterbook();
        create_shm_conf();


        /*-------------------------------*/
        /*  CREATING THE QUEUES          */
        /*-------------------------------*/
        create_users_msg_queue();
        create_nodes_msg_queue();
        create_master_msg_report_queue();

        /*-------------------------*/
        /*  CREATION OF PROCESSES  */
        /*-------------------------*/

        DEBUG_BLOCK_ACTION_START("PROC GENERATION");
        /* Crating users*/
        if (create_users_proc(users_pids, users_queues_ids) < 0) {
            ERROR_MESSAGE("IMPOSSIBLE TO CREATE USERS PROC");
        }
        /* Creating nodes*/
        if (create_nodes_proc(nodes_pids, nodes_queues_ids) < 0) {
            ERROR_MESSAGE("IMPOSSIBLE TO CREATE NODES PROC");
        }
        /**
         * Generate the array of friends for each nodes and save it into a local array
         */
        DEBUG_BLOCK_ACTION_END();
        DEBUG_NOTIFY_ACTIVITY_RUNNING("SHM INITIALIZING...");
        /*-------------------------------*/
        /*  INITIALIZING THE SHM         */
        /*-------------------------------*/
        int *nodes_friends = malloc(sizeof(int) * nodes_queues_ids[0] +1);
        generate_nodes_friends_array(nodes_friends, nodes_queues_ids);
        if (shm_conf_create(shm_conf_pointer, users_pids, users_queues_ids, nodes_pids, nodes_queues_ids,
                            nodes_friends) < 0) {
            ERROR_EXIT_SEQUENCE_MAIN("FAILED ON SHM_CONF INITIALIZING");
        }
        free(nodes_queues_ids);
        free(nodes_pids);
        free(users_pids);
        free(users_queues_ids);
        free(nodes_friends);
        DEBUG_NOTIFY_ACTIVITY_DONE("SHM INITIALIZING DONE");

#ifdef DEBUG_MAIN
        shm_conf_print(shm_conf_pointer);
#endif
        DEBUG_BLOCK_ACTION_START("WAITING CHILDREN");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO WAIT ON SEM_START");
        }
        DEBUG_MESSAGE("WAITING DONE");
        DEBUG_BLOCK_ACTION_END();
        alarm(1);
        while (simulation_end < 0) {
            int msg_rep_value = check_msg_report(&msg_repo, msg_report_id_master, proc_list);
            if (check_runnability() == FALSE || msg_rep_value < 0) {
                /*If a message arrive make the knowledge*/
                ERROR_MESSAGE("IMPOSSIBLE TO RUN SIMULATION");
                end_simulation();
            } else if (msg_rep_value == 1) {
                /**TODO: CHECK IF THE MESSAGE IS CORRECT, AND RUN A NEW NODE AND SEND HIM THE GIVEN TRANSACTION*/
                int new_node_id = shm_conf_pointer->nodes_snapshots[shm_conf_pointer->nodes_snapshots[0][0]][1] +
                                  DELTA_NODE_MSG_TYPE;
                int new_node_pid = create_node_proc(new_node_id);
                int new_node_friends = rand_int_n_exclude(new_node_id, new_node_id,
                                                          shm_conf_pointer->nodes_snapshots[0][0]);
                shm_conf_add_node(shm_conf_pointer, new_node_pid, new_node_id, new_node_friends);
                struct node_msg node_msg;
                node_msg.t.hops = 0;
                node_msg_snd(msg_report_id_nodes, &node_msg, MSG_TRANSACTION_TYPE, &msg_repo.t, main_pid, TRUE,
                             simulation_conf.so_retry, new_node_id);
                /*Send the signal SIGUSR1 to all the nodes*/
                send_sig_to_all_nodes(proc_list, SIGUSR1, TRUE);
                send_msg_to_all_nodes(new_node_id, simulation_conf.so_retry, proc_list,
                                      shm_conf_pointer->nodes_snapshots[0][0], TRUE);
            } else if (msg_rep_value == 2) {
                printf("\n");
                printf("called\n");
                master_msg_report_print(&msg_repo);
            }
        }
        end_simulation();
    }
    free_mem();
    free_sysVar();
    exit(0);
}

int create_users_proc(int *users_pids, int *users_queues_ids) {
    pid_t user_pid;
    int i = 0;
    int queue_id = DELTA_USER_MSG_TYPE;
    if (users_pids == NULL || users_queues_ids == NULL) {
        return -1;
    }
    users_pids[0] = 0;
    users_queues_ids[0] = 0;
    for (; i < simulation_conf.so_user_num; i++) {
        char *argv_user[] = {PATH_TO_USER, NULL, NULL}; /*Future addon*/
        argv_user[1] = (char *) malloc(11 * sizeof(char));
        switch (user_pid = fork()) {
            case -1:
                if (argv_user[1] != NULL)
                    free(argv_user[1]);
                if (argv_user[2] != NULL)
                    free(argv_user[2]);
                return -1;
            case 0:                                    /*kid*/
                sprintf(argv_user[1], "%d", queue_id); /*Let the user know is position*/
                execve(argv_user[0], argv_user, NULL);
                ERROR_MESSAGE("IMPOSSIBLE TO CREATE A USER");
                return -1;
            default: /*father*/
                users_queues_ids[i + 1] = queue_id;
                users_pids[i + 1] = user_pid;
                users_pids[0] += 1;
                users_queues_ids[0] += 1;
                Proc p = proc_create(user_pid, queue_id, PROC_STATE_RUNNING, PROC_TYPE_USER, -1);
                insert_in_list(proc_list, p);
                /*Free if utilized pointers to argv*/
                if (argv_user[1] != NULL)
                    free(argv_user[1]);
                if (argv_user[2] != NULL)
                    free(argv_user[2]);
                queue_id += DELTA_USER_MSG_TYPE;
                DEBUG_MESSAGE("USER CREATED");
                break;
        }
    }
    return 0;
}

int create_nodes_proc(int *nodes_pids, int *nodes_queues_ids) {
    pid_t node_pid;
    int i = 0;
    int queue_id = DELTA_NODE_MSG_TYPE;
    if (nodes_pids == NULL || nodes_queues_ids == NULL) {
        return -1;
    }
    nodes_pids[0] = 0;
    nodes_queues_ids[0] = 0;
    for (; i < simulation_conf.so_nodes_num; i++) {
        char *argv_node[] = {PATH_TO_NODE, NULL, NULL}; /*Future addon*/
        argv_node[1] = (char *) malloc(11 * sizeof(char));/**< 11 is the number of characters INT_MAX+1*/
        switch (node_pid = fork()) {
            case -1:
                if (argv_node[1] != NULL)
                    free(argv_node[1]);
                if (argv_node[2] != NULL)
                    free(argv_node[2]);
                return -1;
            case 0:                                    /*kid*/
                sprintf(argv_node[1], "%d", queue_id); /*Let the user know is position*/
                execve(argv_node[0], argv_node, NULL);
                ERROR_MESSAGE("IMPOSSIBLE TO CREATE A NODE");
                return -1;
            default: /*father*/
                nodes_queues_ids[i + 1] = queue_id;
                nodes_pids[i + 1] = node_pid;
                nodes_pids[0] += 1;
                nodes_queues_ids[0] += 1;
                Proc p = proc_create(node_pid, queue_id, PROC_STATE_RUNNING, PROC_TYPE_NODE, -1);
                insert_in_list(proc_list, p);
                /*Free if utilized pointers to argv*/
                if (argv_node[1] != NULL)
                    free(argv_node[1]);
                if (argv_node[2] != NULL)
                    free(argv_node[2]);
                queue_id += DELTA_NODE_MSG_TYPE;
                DEBUG_MESSAGE("NODE CREATED");
                break;
        }
    }
    return 0;
}

void set_signal_handlers(struct sigaction sa) {
#ifdef DEBUG_MAIN
    DEBUG_BLOCK_ACTION_START("SIGNAL HANDLERS");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("SETTING SIGNALS HANDLERS...");
#endif
    memset(&sa, 0, sizeof(sa)); /*initialize the structure*/
    sa.sa_handler = signals_handler;
    if (sigaction(SIGTSTP, &sa, NULL) < 0 ||
        sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0) {
        ERROR_MESSAGE("ERROR SETTING SIGNAL HANDLERS");
        EXIT_PROCEDURE_MAIN(EXIT_FAILURE);
    }
#ifdef DEBUG_MAIN
    DEBUG_NOTIFY_ACTIVITY_DONE("SETTING HANDLERS DONE");
    DEBUG_BLOCK_ACTION_END();
#endif
}

void signals_handler(int signum) {
#ifdef DEBUG_MAIN
    DEBUG_SIGNAL("SIGNAL RECEIVED", signum);
#endif
    last_signal = signum;
    int old_errno;
    static int num_inv = 0;
    old_errno = errno;
    switch (signum) {
        case SIGINT:
        case SIGTERM:
            if (getpid() == main_pid) {
                EXIT_PROCEDURE_MAIN(0);
            } else {
                exit(0);
            }
        case SIGALRM:
            if (getpid() == main_pid) {
                num_inv++;
                /*request info from kids */
                update_kids_info();
                print_info();
                /*Printing infos*/
                if (num_inv == simulation_conf.so_sim_sec) {
                    simulation_end = SIMULATION_END_BY_TIME;
                    end_simulation();
                    break;
                } else
                    alarm(1);
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
    if (semctl(semaphore_start_id, 0, SETVAL, simulation_conf.so_user_num + (simulation_conf.so_nodes_num)) <
        0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO INITIALISE SEMAPHORE START CHILDREN");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("INITIALIZATION OF START_SEMAPHORE CHILDREN DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF MASTEBOOK ACCESS SEM....");

    semaphore_masterbook_id = semget(SEMAPHORE_MASTER_BOOK_ACCESS_KEY, SO_REGISTRY_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    if (semaphore_masterbook_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE MASTERBOOK SEM");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF MASTEBOOK ACCESS SEM DONE");
    DEBUG_BLOCK_ACTION_END();

    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF TO_FILL SEM....");
    semaphore_to_fill_id = semget(SEMAPHORE_MASTER_BOOK_TO_FILL_KEY, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semaphore_start_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE TO_FILL SEM");
    }
    semctl(semaphore_to_fill_id, 0, SETVAL, 1);
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF TO_FILL DONE");
}

void wait_kids() {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("WAITING KIDS...");
    saving_private_ryan(proc_list, msg_report_id_master);
    DEBUG_NOTIFY_ACTIVITY_DONE("WAITING KIDS DONE");
}

void kill_kids() {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("KILLING KIDS...");
    struct master_msg_report msg_rep;
    check_msg_report(&msg_rep, msg_report_id_master, proc_list);
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
    if (semaphore_masterbook_id >= 0 && semctl(semaphore_masterbook_id, 0, IPC_RMID) < 0)
        ERROR_MESSAGE("REMOVING PROCEDURE FOR MASTERBOOK SEM HAS FAILED");
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING MASTERBOOK SEMAPHORE DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING TO_FILL MASTERBOOK SEMAPHORE...");
    if (semaphore_to_fill_id >= 0 && semctl(semaphore_to_fill_id, 0, IPC_RMID) < 0)
        ERROR_MESSAGE("REMOVING PROCEDURE FOR TO_FILL MASTERBOOK SEM HAS FAILED");
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING TO_FILL MASTERBOOK SEMAPHORE DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING SHM CONF...");
    if (shm_conf_id >= 0 && shmctl(shm_conf_id, IPC_RMID, NULL) < 0) {
        ERROR_MESSAGE("REMOVING PROCEDURE FOR SHM_CONF FAILED");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING SHM CONF DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING SHM MASTERBOOK...");
    if (shm_masterbook_id >= 0 && shmctl(shm_masterbook_id, IPC_RMID, NULL) < 0) {
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
#ifdef DEBUG_MAIN
    DEBUG_BLOCK_ACTION_START("READING CONF");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("READING CONFIGURATION...");
#endif
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
#ifdef DEBUG_MAIN
    DEBUG_NOTIFY_ACTIVITY_DONE("READING CONFIGURATION DONE");
    DEBUG_BLOCK_ACTION_END();
#endif
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

void create_users_msg_queue(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING MSG REPORT QUEUE FOR USERS....");
    msg_report_id_users = msgget(USERS_QUEUE_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if (msg_report_id_users < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE USER MESSAGE QUEUE");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING MSG REPORT QUEUE FOR USERS DONE");
}

void create_nodes_msg_queue(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING MSG REPORT QUEUE FOR NODES...");
    msg_report_id_nodes = msgget(NODES_QUEUE_KEY, IPC_CREAT | IPC_CREAT | 0600);
    if (msg_report_id_nodes < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE NODES MESSAGE QUEUE");
    }
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
    if (shm_book_master_create(shm_masterbook_pointer, semaphore_masterbook_id) == -1) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE MASTERBOOK");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING THE MASTER BOOK DOONE");
    DEBUG_BLOCK_ACTION_END();
}

void print_info(void) {
    printf("============== INFO ============== \n");
    printf("Number of node active: %d\n", get_num_of_user_proc_running(proc_list));
    print_list(proc_list);
    printf("\nTO FILL SHM VALUE: %d\n", shm_masterbook_pointer->to_fill);
    printf("============== END INFO ===========\n");
}

void update_kids_info(void) {
    DEBUG_BLOCK_ACTION_START("UPDATE KIDS INFO");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("UPDATING KIDS INFO....");
    struct master_msg_report *msg_rep = malloc(sizeof(struct master_msg_report));
    if (check_msg_report(msg_rep, msg_report_id_master, proc_list) == 1) {
        ERROR_MESSAGE("MESSAGE TP FULL ACKNOWLEDGED\n");
    }
    int num_msg_to_wait_for = -1;
    num_msg_to_wait_for = send_sig_to_all(proc_list, SIGUSR2);
    if (num_msg_to_wait_for < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO UPDATE KIDS INFO");
    } else if (num_msg_to_wait_for == 0) {
        DEBUG_MESSAGE("NO PROCESS TO UPDATE");
    }
    DEBUG_NOTIFY_ACTIVITY_RUNNING("RETRIVING INFO ...");
    do {
        msg_rep->sender_pid = -1;
        /*TODO: POSSIBLE INFITE WAITING CHECK FOR SIG*/
        master_msg_receive_info(msg_report_id_master, msg_rep);

        if (msg_rep->sender_pid != -1) {
            master_msg_report_print(msg_rep);
#ifdef DEBUG_MAIN
            master_msg_report_print(msg_rep);
#endif
            if (acknowledge(msg_rep, proc_list) == 1) {
                ERROR_MESSAGE("MESSAGE TP FULL ACKNOWLEDGED\n");
            }
            num_msg_to_wait_for--;
        }
    } while (num_msg_to_wait_for > 0);
    DEBUG_NOTIFY_ACTIVITY_DONE("RETRIVING INFO DONE");
}

Bool check_runnability() {
    int num_user_proc_running = get_num_of_user_proc_running(proc_list);
    if (shm_masterbook_pointer->to_fill >= SO_REGISTRY_SIZE) {
        simulation_end = SIMULATION_END_BY_SO_REGISTRY_FULL;
        return FALSE;
    } else if (num_user_proc_running <= 0) {
        simulation_end = SIMULATION_END_BY_NO_PROC_RUNNING;
        return FALSE;
    } else {
        return TRUE;
    }
}

void end_simulation() {
    printf("Simulation %s \n", get_end_simulation_msg());
    kill_kids();
    wait_kids();
    free_mem();
    free_sysVar();
    exit(0);
}

char *get_end_simulation_msg() {
    switch (simulation_end) {
        case SIMULATION_END_BY_USER:
            return "END BY USER";
        case SIMULATION_END_BY_TIME:
            return "END BY TIME";
        case SIMULATION_END_BY_NO_PROC_RUNNING:
            return "END BY NO PROCESS RUNNING";
        case SIMULATION_END_BY_SO_REGISTRY_FULL:
            return "END BY SO REGISTRY FULL";
        case SIMULATION_END_PROPERLY_TERMINATED:
            return "END PROPERLY TERMINATED";
        default:
            return "UNKNOWN";
    }
}

void generate_nodes_friends_array(int *nodes_friends, int *nodes) {
    int i = 1;
    nodes_friends[0] = nodes[0];
    for (; i < nodes[0]; i++) {
        nodes_friends[i] = rand_int_n_exclude(simulation_conf.so_num_friends, i, nodes[0]);
    }
}

int create_node_proc(int new_node_id) {
    printf("\n\n\nCREATING NODE %d\n", new_node_id);
    pid_t new_node_pid;
    char *argv_node[] = {PATH_TO_NODE, NULL, NULL};
    Proc p;
    /*Add the new node id to argv_node and run the new node process*/
    argv_node[1] = (char *) malloc(sizeof(char) * 11);
    sprintf(argv_node[1], "%d", new_node_id);
    switch (new_node_pid = fork()) {
        case -1:
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO FORK NEW NODE PROCESS");
        case 0:
            execv(PATH_TO_NODE, argv_node);
            ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO EXEC NEW NODE PROCESS");
        default:
            p = proc_create(new_node_pid, new_node_id, PROC_STATE_RUNNING, PROC_TYPE_NODE, -1);
            insert_in_list(proc_list, p);
            /*Free the memory allocated for the argv_node*/
            free(argv_node[1]);
            DEBUG_MESSAGE("NEW NODE PROCESS CREATED");
            return new_node_pid;
    }
}

void lock_to_fill_sem(void) {
    while (semaphore_lock(semaphore_to_fill_id, 0) < 0) {
        if (errno == EINTR) {
            continue;
        } else {
            ERROR_EXIT_SEQUENCE_MAIN("ERROR WHILE TRYING TO EXEC LOCK ON TO_FILL ACCESS SEM");
        }
    }
}

void unlock_to_fill_sem(void) {
    while (semaphore_unlock(semaphore_to_fill_id, 0) < 0) {
        if (errno != EINTR) {
            ERROR_EXIT_SEQUENCE_MAIN("ERROR DURING THE UNLOCK OF THE SEMAPHORE");
        }
    }
}
