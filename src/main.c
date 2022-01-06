/*Deprecated: new standard is not implemented for university course constrain*/
#define _GNU_SOURCE
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

#define _FORTIFY_SOURCE 2
/*  LOCAL IMPORT*/

#include "local_lib/headers/glob_vars.h"
#include "local_lib/headers/conf_file.h"
#include "local_lib/headers/boolean.h"
#include "local_lib/headers/semaphore.h"
#include "local_lib/headers/simulation_errors.h"

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

int create_user_proc(pid_t old_pid);

Bool read_conf();

struct conf simulation_conf;
/*Variabili di SYS*/
/*TODO: message transaction queue for single user_transaction auto incremento with ## macro*/
int simulation_end = 0;
int msg_transaction_reports = -1; /*Identifier for message queue*/
int semaphore_start_id = -1;
/*Variabili Globali*/
pid_t main_pid;
struct processes_info_list *proc_list;

int main() {
    /*semctl(3, 0, IPC_RMID);TODO: Remove*/
    /************************************
     *      CONFIGURATION FASE
     * ***********************************/
    main_pid = getpid();
    if (read_conf() == TRUE) {
        int i; /*  utility index */
        struct sigaction sa; /*Structure for handling signals*/
        set_signal_handlers(sa);
        create_semaphores();
        create_masterbook();

        /*************************************
         *  CREATION OF CHILD PROCESSES FASE *
         * ***********************************/

        /*-------------------------*/
        /*  CREAZINE DEI PROC USER *
        /*-------------------------*/

        DEBUG_BLOCK_ACTION_START("PROC GENERATION");
        for (i = 0; i < simulation_conf.so_user_num; i++)
            if (create_user_proc(0) < 0)ERROR_MESSAGE("IMPOSSIBLE TO CREATE USER_PROC");

        DEBUG_MESSAGE("PROCESS USER GENERATED");

        DEBUG_BLOCK_ACTION_END();

        if (semaphore_wait_for_sinc(semaphore_start_id, 0) < 0) {
            ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO WAIT ON SEM_START");
        }
    }
    free_sysVar();
    free_mem();
    return 0;
}

/**
 *  Create a new user proc
 * @param old_pid the pid of the proc to be replaced TODO: possible adding funtionality 0 = no replacement
 * @return
 */
int create_user_proc(pid_t old_pid) {
    char *argv_user[] = {PATH_TO_USER, NULL, NULL}; /*TODO: verify if needed*/
    pid_t kid_pid;
    switch (kid_pid = fork()) {
        case -1:
            return -1;
        case 0: /*  kid   */
            execve(argv_user[0], argv_user, NULL);
            ERROR_MESSAGE("IMPOSSIBLE TO CREATE A USER");
            return -1;
        default: /*  parent  */
            list_set_state(proc_list, old_pid, PROC_INFO_STATE_TERMINATED);
            union ProcInfo proc_info;
            UserProcInfo user_info;
            proc_info.user_proc = user_info;
            proc_list = insert_in_list(proc_list, kid_pid, PROC_TYPE_USER, proc_info);
            /*Free if utilized pointers to argv*/
            if (argv_user[1] != NULL) free(argv_user[1]);
            if (argv_user[2] != NULL) free(argv_user[2]);
            DEBUG_MESSAGE("USER CREATED");
            break;
    }
    return 0;
}

void set_signal_handlers(struct sigaction sa) {
    DEBUG_BLOCK_ACTION_START("SIGNAL HANDLERS");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("Setting Signals Handlers...");
    memset(&sa, 0, sizeof(sa));/*initialize the structure*/
    sa.sa_handler = signals_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGALRM, &sa, NULL) < 0) {
        ERROR_MESSAGE("Errore Setting Signal Handlers");
        EXIT_PROCEDURE_MAIN(EXIT_FAILURE);
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("Setting )free_sysVar_userSignals Handlers COMPLETED");
    DEBUG_BLOCK_ACTION_END();
}

/**
 * Handler for the signals SIGINT, SIGTERM, SIGALARM
 * @param signum
 */
void signals_handler(int signum) { /*TODO: Scrivere implementazione*/
    int old_errno;
    static int num_inv = 0;
    DEBUG_SIGNAL("SIGNAL RECIVED", signum);
    switch (signum) {
        case SIGINT:
        case SIGTERM:
            if (getpid() == main_pid)EXIT_PROCEDURE_MAIN(0);
            exit(0);
        case SIGALRM:
            if (getpid() == main_pid) {
                num_inv++;
                if (num_inv == simulation_conf.so_sim_sec) simulation_end = 1;
                else alarm(1);
                /*TODO: METTO IN PAUSA I NODI vedere se mettere anche in pausa i processi user*/
            }
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
    printf("%d", simulation_conf.so_user_num);
    if (semctl(semaphore_start_id, 0, SETVAL, simulation_conf.so_user_num + simulation_conf.so_nodes_num) <
        0) {

        ERROR_EXIT_SEQUENCE_MAIN("IMPOSSIBLE TO INITIALISE SEMAPHORE START CHILDREN");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("INITIALIZATION OF START_SEMAPHORE CHILDREN DONE");
    DEBUG_BLOCK_ACTION_END();
}

void create_masterbook() {
    /*TODO:da implementare create_masterbook*/
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
Bool read_conf() {
    DEBUG_BLOCK_ACTION_START("READING CONF")
    DEBUG_NOTIFY_ACTIVITY_RUNNING("READING CONFIGURATION...");
    switch (load_configuration(&simulation_conf)) {
        case 0:
            break;
        case -1:
        ERROR_EXIT_SEQUENCE_MAIN(" DURING CONF. LOADING: MISSING FILE OR EMPTY");
        case -2:
        ERROR_EXIT_SEQUENCE_MAIN(" DURING CONF. LOADING: BROKEN SIMULTATION LOGIC, CHECK CONF. VALUE");
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