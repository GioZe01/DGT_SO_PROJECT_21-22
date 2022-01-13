/*Deprecated: new standard is not implemented for university course constrain*/
#define _GNU_SOURCE
#define _OPEN_SYS_ITOA_EXT
/*If this macro is defined to 1, security hardening is added to various library functions. If def
 * ined to 2, even stricter checks are applied. If defined to 3, the GNU C Library may also use checks that may have
 * an additional performance*/
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

#ifdef DEBUG

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

#include "local_lib/headers/process_info_list.h"

/* Funzioni di supporto al main */
void set_signal_handlers(struct sigaction sa);

void signals_handler(int signum);

void create_semaphores(void);

void create_masterbook(void);

int create_users_proc(void);

int create_nodes_proc(void);

Bool read_conf(void);

void notify_users_of_pid_to_id(void);

void notify_nodes_pid_to_id(void);

void create_msg_report_queue(void);

int check_msg_report(struct master_msg_report *msgReport);

/* Variabili */
struct conf simulation_conf;
int *users_id_to_pid; /*in 0 position is saved the actual size of the array of id saved in the pointer*/
int *nodes_id_to_pid; /*in 0 position is saved the actual size */
struct processes_info_list *proc_list;

int simulation_end = 0;
int msg_report_id = -1; /*Identifier for message queue*/
int semaphore_start_id = -1;
pid_t main_pid;

int main() {
    /*   semctl(3, 0, IPC_RMID); TODO: Remove*/
    main_pid = getpid();
    if (read_conf() == TRUE) {
        /*  Local Var Declaration   */
        int i; /*  utility index */
        struct sigaction sa; /*Structure for handling signals */
        struct master_msg_report msg_repo;
        /* Pointers allocation  */
        users_id_to_pid = (int *) malloc(sizeof(int) * simulation_conf.so_user_num);
        nodes_id_to_pid = (int *) malloc(sizeof(int) * simulation_conf.so_nodes_num);
        /************************************
         *      CONFIGURATION FASE
         * ***********************************/

        set_signal_handlers(sa);
        create_semaphores();
        create_masterbook();
        create_msg_report_queue();

        /*************************************
         *  CREATION OF CHILD PROCESSES FASE *
         * ***********************************/

        /*-------------------------*/
        /*  CREAZINE DEI PROC USER *
        /*-------------------------*/

        DEBUG_BLOCK_ACTION_START("PROC GENERATION");
        /* Crating users*/
        if (create_users_proc() < 0) { ERROR_MESSAGE("IMPOSSIBLE TO CREATE USERS PROC"); }
        /* Creating nodes*/
        if (create_nodes_proc() < 0) { ERROR_MESSAGE("IMPOSSIBLE TO CREATE NODES PROC"); }
        DEBUG_BLOCK_ACTION_END();

        DEBUG_NOTIFY_ACTIVITY_RUNNING("SHRINKING ID_TO_PID REF...");
        if (users_id_to_pid[0] < (simulation_conf.so_user_num - REALLOC_MARGIN)) {
            users_id_to_pid = realloc(users_id_to_pid, sizeof(int) * users_id_to_pid[0]);
        }
        if (nodes_id_to_pid[0]< (simulation_conf.so_nodes_num - REALLOC_MARGIN)){
            nodes_id_to_pid = realloc(nodes_id_to_pid, sizeof(int) * users_id_to_pid[0]);
        }
        DEBUG_NOTIFY_ACTIVITY_DONE("SHRINKING ID_TO_PID REF DONE");
#ifdef DEBUG
        printf("\n==========DEBUG INFO TABLE==========\n");
        printf("# Users generated: %d\n", users_id_to_pid[0]);
        printf("# Nodes generated: %d\n", nodes_id_to_pid[0]);
        printf("======================================\n");
#endif
        DEBUG_MESSAGE("PROCESSES USERS GENERATED");
        DEBUG_BLOCK_ACTION_START("WAITING CHILDREN");
        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO WAIT ON SEM_START");
        }

        DEBUG_MESSAGE("WAITING DONE");

        DEBUG_BLOCK_ACTION_END();

        DEBUG_BLOCK_ACTION_START("NOTIFY USERS AND NODES");
        notify_users_of_pid_to_id();
        notify_nodes_pid_to_id();
        DEBUG_BLOCK_ACTION_END();
        while (simulation_end != 1) {
            if (check_msg_report(&msg_repo) < 0) {
                /*If a message arrive make the knowledge*/
            }
        }
        printf("====TIME FINISHED====\n");
        kill_kids();
        /*TODO: verifica messaggi di report*/
        wait_kids();
        /*TODO: final printing*/
    }
    free_sysVar();
    free_mem();
    return 0;
}

/**
 *  Create a new user proc
 * @return -1 if fail. 0 otherwise
 */
int create_users_proc(void) {
    char *argv_user[] = {PATH_TO_USER, NULL}; /*Future addon*/
    pid_t user_pid;
    int i;
    users_id_to_pid[0] = 0;
    for (i = 0; i < simulation_conf.so_user_num; i++) {
        switch (user_pid = fork()) {
            case -1:
                return -1;
            case 0: /*kid*/
                execve(argv_user[0], argv_user, NULL);
                ERROR_MESSAGE("IMPOSSIBLE TO CREATE A USER");
                return -1;
            default: /*father*/
                users_id_to_pid[i + 1] = user_pid;
                users_id_to_pid[0] += 1;
                proc_list = insert_in_list(proc_list, user_pid, PROC_TYPE_USER);
                /*Free if utilized pointers to argv*/
                if (argv_user[1] != NULL) free(argv_user[1]);
                if (argv_user[2] != NULL) free(argv_user[2]);
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
int create_nodes_proc(void) {
    char *argv_node[] = {PATH_TO_NODE, NULL};
    pid_t node_pid;
    int i;
    nodes_id_to_pid[0] = 0;
    for (i = 0; i < simulation_conf.so_nodes_num; i++) {
        switch (node_pid = fork()) {
            case -1:
                return -1;
            case 0: /*kid*/
                execve(argv_node[0], argv_node, NULL);
                ERROR_MESSAGE("IMPOSSIBLE TO CREATE A USER");
                return -1;
            default:/*father*/
                nodes_id_to_pid[i + 1] = node_pid;
                nodes_id_to_pid[0] += 1;
                proc_list = insert_in_list(proc_list, node_pid, PROC_TYPE_NODE);
                /*Free if utilized pointer to argv*/
                if (argv_node[1] != NULL) free(argv_node[1]);
                if (argv_node[2] != NULL) free(argv_node[2]);
                DEBUG_MESSAGE("NODE CREATED");
                break;
        }
    }
    return 0;
}

void notify_users_of_pid_to_id(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("SENDING INFORMATION TO USERS...");
    struct processes_info_list *list = proc_list;
    struct user_msg msg;
    int user_queue_id;
    if (node_msg_create(&msg, MSG_CONFIG_TYPE, main_pid, users_id_to_pid) < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE MESSAGE");
    }
#ifdef DEBUG
    user_msg_print(&msg);
#endif
    for (; list != NULL; list = list->next) {
        if (list->proc_type == PROC_TYPE_USER) {
            user_queue_id = msgget(proc_list->pid, 0600);
            if (user_queue_id < 0) {
                ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO RETRIEVE QUEUE");
            }
            if (user_msg_snd(user_queue_id, &msg, MSG_CONFIG_TYPE, users_id_to_pid, main_pid, FALSE) < 0) {
                ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO SEND MESSAGE ERROR");
            }
        }
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("SENDING INFORMATION TO USERS DONE");
}

void notify_nodes_pid_to_id(void) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("SENDING INFORMATION TO USERS...");
    struct processes_info_list *list = proc_list;
    struct user_msg msg;
    int user_queue_id;
    if (user_msg_create(&msg, MSG_CONFIG_TYPE, main_pid, users_id_to_pid) < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE MESSAGE");
    }
#ifdef DEBUG
    user_msg_print(&msg);
#endif
    for (; list != NULL; list = list->next) {
        if (list->proc_type == PROC_TYPE_NODE) {
            user_queue_id = msgget(proc_list->pid, 0600);
            if (user_queue_id < 0) {
                ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO RETRIEVE QUEUE");
            }
            if (user_msg_snd(user_queue_id, &msg, MSG_CONFIG_TYPE, users_id_to_pid, main_pid, FALSE) < 0) {
                ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO SEND MESSAGE ERROR");
            }
        }
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("SENDING INFORMATION TO USERS DONE");
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
        ERROR_MESSAGE("ERRORE SETTING SIGNAL HANDLERS");
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
        case SIGTSTP:
        case SIGTERM:
            if (getpid() == main_pid) { EXIT_PROCEDURE_MAIN(0); }
            else { exit(0); }
        case SIGALRM:
            if (getpid() == main_pid) {
                num_inv++;
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
    DEBUG_BLOCK_ACTION_START("CREATE START_SEM");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATION OF START_SEMAPHORE CHILDREN....");

    semaphore_start_id = semget(SEMAPHORE_SINC_KEY_START, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semaphore_start_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE START_SEMAPHORE");
    }

    DEBUG_NOTIFY_ACTIVITY_DONE("CREATION OF START_SEMAPHORE CHILDREN DONE");

    DEBUG_NOTIFY_ACTIVITY_RUNNING("INITIALIZATION OF START_SEMAPHORE CHILDREN....");
    if (semctl(semaphore_start_id, 0, SETVAL, simulation_conf.so_user_num /*TODO+ simulation_conf.so_nodes_num*/) <
        0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO INITIALISE SEMAPHORE START CHILDREN");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("INITIALIZATION OF START_SEMAPHORE CHILDREN DONE");
    DEBUG_BLOCK_ACTION_END();
}

void create_masterbook() {
    DEBUG_BLOCK_ACTION_START("CREATE MASTERBOOK");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING THE MASTER_BOOK....");
    /*TODO:da implementare create_masterbook*/
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING THE MASTER BOOK DOONE");
    DEBUG_BLOCK_ACTION_END();
}

/*  IMPLEMENTATION OF PROC_INFO_LIST METHOD*/
void wait_kids() {
    struct processes_info_list *proc = proc_list;
    for (; proc != NULL; proc = proc->next)
        if (proc->proc_state == PROC_INFO_STATE_RUNNING) {
            waitpid(proc->pid, NULL, 0);
            proc->proc_state = PROC_INFO_STATE_TERMINATED;
        }
}

void kill_kids() {
    struct processes_info_list *proc = proc_list;
    for (; proc != NULL; proc = proc->next)
        if (proc->proc_state == PROC_INFO_STATE_RUNNING)
            if (kill(proc->pid, SIGINT) >= 0 || errno == ESRCH)
                /**
                 * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
                 * the termination has not been read by main, in this case need wait on the proc to update the proc-list
                 * state
                 */
                DEBUG_MESSAGE("PROC KILLED");
            else {
                if (errno == EINTR) continue;
                ERROR_MESSAGE("IMPOSSIBLE TO SEND TERMINATION SIGNAL TO KID");
            }
}

void free_mem() {
    list_free(proc_list);
}

void free_sysVar() {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING STARTING SEMAPHORE...");
    if (semaphore_start_id >= 0 && semctl(semaphore_start_id, 0, IPC_RMID) < 0)
        ERROR_MESSAGE("REMOVING PROCEDURE FOR START_SEM HAS FAILED");
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING STARTING SEMAPHORE DONE");

}

/**
 * Load and read the configuration, in case of error during loading close the proc. with EXIT_FAILURE
 * @return TRUE if ALL OK
 */
Bool read_conf(void) {
    DEBUG_BLOCK_ACTION_START("READING CONF")
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
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("READING CONFIGURATION DONE");
    DEBUG_BLOCK_ACTION_END();
    return TRUE;
}

void create_msg_report_queue(void) {
    DEBUG_BLOCK_ACTION_START("MSG REPORT QUEUE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING THE MESSAGE REPORT QUEUE...");
    msg_report_id = msgget(MASTER_QUEUE_KEY, IPC_CREAT | IPC_EXCL | 0600);
    if (msg_report_id < 0) {
        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO CREATE THE MESSAGE REPORT QUEUE");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING THE MESSAGE REPORT QUEUE DONE");
    DEBUG_BLOCK_ACTION_END();

}

int check_msg_report(struct master_msg_report *msg_report) {
    struct msqid_ds msg_rep_info;
    if (msgctl(msg_report_id, IPC_STAT, &msg_rep_info) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE MESSAGE QUEUE INFO");
        return -1;
    } else {
        /*fetching all msg if present*/
        if (msg_rep_info.msg_qnum != 0 && msgrcv(msg_report_id, msg_report, sizeof(*msg_report) - sizeof(long), 0, 0)) {
            /*make_knowledge*/
        }
        return 0;
    }
}