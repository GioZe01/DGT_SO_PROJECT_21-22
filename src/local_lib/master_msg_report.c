#define _GNU_SOURCE
/* Std */
#include <stdio.h>
#include <errno.h>
/* Sys */
#include <sys/msg.h>
/* Local */
#include "headers/master_msg_report.h"
#include "headers/glob.h"
#include "headers/process_info_list.h"
#include "headers/simulation_errors.h"
#include "headers/debug_utility.h"
#include "headers/transaction_list.h"

void
master_msg_report_create(struct master_msg_report *self, long type, long proc_type, pid_t sender_pid, short int state,
                         float budget, struct Transaction t) {
    self->t = t;
    self->type = type;
    self->proc_type = proc_type;
    self->sender_pid = sender_pid;
    self->state = state;
    self->budget = budget;
}

void master_msg_report_print(const struct master_msg_report *self) {
    char state_string[80];
    switch (self->type) {
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
        case IMPOSSIBLE_TO_ACQUIRE_SEMAPHORE:
        case IMPOSSIBLE_TO_LOAD_CONFIGURATION:
        case IMPOSSIBLE_TO_COMUNICATE_WITH_QUEUE:
        case IMPOSSIBLE_TO_GENERATE_TRANSACTION:
        case IMPOSSIBLE_TO_CONNECT_TO_SHM:
        case MAX_FAILURE_SHM_REACHED:
            switch (self->state) {
                case PROC_STATE_WAITING:
                case PROC_STATE_NODE_SERV_TRANS:
                    printf("[MASTER_MSG_REPORT]  := msg_type: %s%s%s%s | porc_type: %s | sender: %d | state: %s%s%d%s | budget : %f\n",
                           COLOR_RESET_ANSI_CODE,
                           COLOR_RED_ANSI_CODE, from_type_to_string(self->type), COLOR_RESET_ANSI_CODE,
                           from_proctype_to_string(self->proc_type), self->sender_pid, COLOR_RESET_ANSI_CODE,
                           COLOR_YELLOW_ANSI_CODE, self->state, COLOR_RESET_ANSI_CODE, self->budget);
                    break;
                default:
                    printf("[MASTER_MSG_REPORT]  := msg_type: %s%s%s%s | porc_type: %s  | sender: %d | state: %d | budget: %f\n",
                           COLOR_RESET_ANSI_CODE,
                           COLOR_RED_ANSI_CODE, from_type_to_string(self->type), COLOR_RESET_ANSI_CODE,
                           from_proctype_to_string(self->proc_type), self->sender_pid, self->state, self->budget);
                    break;
            }
            break;
        default:
            from_procstate_to_string(self->state, state_string);
            printf("[MASTER_MSG_REPORT]  := msg_type: %s | proc_type: %s | sender: %d | state: %s | budget: %f\n",
                   from_type_to_string(self->type), from_proctype_to_string(self->proc_type), self->sender_pid,
                   state_string, self->budget);
    }
}

void from_procstate_to_string(int state, char *string) {
    strcpy(string, COLOR_RESET_ANSI_CODE);
    switch (state) {
        case PROC_STATE_WAITING:
            strcat(string, COLOR_YELLOW_ANSI_CODE);
            strcat(string, "WAITING");
            break;
        case PROC_STATE_RUNNING:
            strcat(string, COLOR_GREEN_ANSI_CODE);
            strcat(string, "RUNNING");
            break;
        case PROC_STATE_TERMINATED:
            strcat(string, COLOR_RED_ANSI_CODE);
            strcat(string, "ENDED");
            break;
        case PROC_STATE_INIT:
            strcat(string, "INIT");
            break;
        default:
            strcat(string, "ERROR");
            break;
    }
    strcat(string, COLOR_RESET_ANSI_CODE);
}

char *from_proctype_to_string(long proc_type) {
    switch (proc_type) {
        case USER:
            return "USER";
        case NODE:
            return "NODE";
        default:
            return "UNDEFINED PROC TYPE";
    }
}

char *from_type_to_string(long type) {
    switch (type) {
        case TERMINATION_END_CORRECTLY:
            return "TERMINATION END CORRECTLY";
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
            return "IMPOSSIBLE TO SEND TRANSACTION";
        case IMPOSSIBLE_TO_GENERATE_TRANSACTION:
            return "IMPOSSIBLE TO GENERATE TRANSACTION";
        case IMPOSSIBLE_TO_ACQUIRE_SEMAPHORE:
            return "IMPOSSIBLE TO ACQUIRE SEMAPHORE";
        case IMPOSSIBLE_TO_LOAD_CONFIGURATION:
            return "IMPOSSIBLE TO LOAD CONFIGURATION";
        case IMPOSSIBLE_TO_COMUNICATE_WITH_QUEUE:
            return "IMPOSSIBLE TO COMUNICATE WITH QUEUE";
        case IMPOSSIBLE_TO_CONNECT_TO_SHM:
            return "IMPOSSIBLE TO CONNECT TO SHM";
        case SIGNALS_OF_TERM_RECEIVED:
            return "SIGNALS OF TERM RECEIVED";
        case INFO_BUDGET:
            return "INFO BUDGET";
        case UNUSED_PROC:
            return "UNUSED PROCESS";
        case TP_FULL:
            return "TP_FULL";
        case MAX_FAILURE_SHM_REACHED:
            return "MAX FAILURE SHM REACHED";
        default:
            return "";
    }
}

int
master_msg_send(int id, struct master_msg_report *self, long type, long proc_type, pid_t sender_pid, short int state,
                Bool create, float budget, struct Transaction *t) {
    if (create == TRUE) {
        master_msg_report_create(self, type, proc_type, sender_pid, state, budget, *t);
    }
#ifdef SHOW_MASTER_MSG_REPORT
    char string[80];
    from_procstate_to_string(self->state, string);
    printf("\nqueue id: %d | process_type: %s | process_state: %s \n", id, from_proctype_to_string(self->proc_type), string);
#endif
    while (msgsnd(id, self, sizeof(struct master_msg_report) - sizeof(long), 0) < 0) {
        if (errno != EINTR)
            return -1;
    }
    return 0;
}

int master_msg_receive(int id, struct master_msg_report *self) {
    if (msgrcv(id, self, sizeof(struct master_msg_report) - sizeof(long), 0 /*all incoming messages*/, IPC_NOWAIT) <
        0) {
        if (errno == ENOMSG) {
            return -2;
        }
        return -1;
    }
    return 0;
}

int master_msg_receive_info(int id, struct master_msg_report *self) {
    if (msgrcv(id, self, sizeof(struct master_msg_report) - sizeof(long), INFO_BUDGET, IPC_NOWAIT) < 0) {
        if (errno == ENOMSG) {
            return -2;
        }
        return -1;
    }
    return 0;
}

int acknowledge(struct master_msg_report *self, ProcList list) {
    long msg_type = self->type;
    switch (msg_type) {
        case TERMINATION_END_CORRECTLY:
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
        case IMPOSSIBLE_TO_CONNECT_TO_SHM:
        case IMPOSSIBLE_TO_LOAD_CONFIGURATION:
        case IMPOSSIBLE_TO_ACQUIRE_SEMAPHORE:
        case IMPOSSIBLE_TO_COMUNICATE_WITH_QUEUE:
        case IMPOSSIBLE_TO_GENERATE_TRANSACTION:
        case SIGNALS_OF_TERM_RECEIVED:
        case UNUSED_PROC:
        case INFO_BUDGET:
            if (update_proc(list, self->sender_pid, self->budget, self->state) == -2) {
                return -2;
            }
            break;
        case TP_FULL:
            return 1;
        default:
            return -1;
    }
    return 0;
}

int check_msg_report(struct master_msg_report *msg_report, int msg_report_id_master, ProcList proc_list) {
    struct msqid_ds msg_rep_info;
    int ris;
    if (msgctl(msg_report_id_master, IPC_STAT, &msg_rep_info) < 0) {
        DEBUG_ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE MESSAGE QUEUE INFO");
        return -1;
    } else {
        /*fetching all msg if present*/
        while (msg_rep_info.msg_qnum != 0 &&
               msgrcv(msg_report_id_master, msg_report, sizeof(*msg_report) - sizeof(long), 0, 0) > 0 &&
               msgctl(msg_report_id_master, IPC_STAT, &msg_rep_info) >= 0) {
            ris = acknowledge(msg_report, proc_list);

            if (ris == -1) {
                ERROR_MESSAGE("IMPOSSIBLE TO MAKE THE ACKNOWLEDGE OF MASTER MESSAGE");
            } else if (ris == 1) {
                return 1;
            }
        }
        if (msg_rep_info.msg_qnum == 0) {
            return 0;
        } else {
            return 2;
        }
    }
}
