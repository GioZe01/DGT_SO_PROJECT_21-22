#define _GNU_SOURCE
/* Sys */
#include <stdio.h>
#include <sys/msg.h>
#include <errno.h>
/* Local */
#include "headers/master_msg_report.h"
#include "headers/glob.h"
#include "headers/process_info_list.h"
#include "headers/simulation_errors.h"
#include "headers/debug_utility.h"

void master_msg_report_create(struct master_msg_report *self, long type,long proc_type, pid_t sender_pid, short int state, float budget) {
    self->type = type;
    self->proc_type = proc_type;
    self->sender_pid = sender_pid;
    self->state = state;
    if (type == INFO_BUDGET){
        self->budget = budget;
    }
    self->budget = 0;
}

void master_msg_report_print(const struct master_msg_report *self) {
    switch (self->type) {
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
        case IMPOSSIBLE_TO_CONNECT_TO_SHM:
            switch (self->state) {
                case PROC_STATE_WAITING:
                case PROC_STATE_NODE_SERV_TRANS:
                    printf("[MASTER_MSG_REPORT]  := type: %s%s%ld%s | sender: %d | state: %s%s%d%s \n", COLOR_RESET_ANSI_CODE,
                           COLOR_RED_ANSI_CODE, self->type, COLOR_RESET_ANSI_CODE,self-> sender_pid,COLOR_RESET_ANSI_CODE, COLOR_YELLOW_ANSI_CODE ,self->state, COLOR_RESET_ANSI_CODE);
                    break;
                default:
                    printf("[MASTER_MSG_REPORT]  := type: %s%s%ld%s | sender: %d | state: %d \n", COLOR_RESET_ANSI_CODE,
                           COLOR_RED_ANSI_CODE, self->type, COLOR_RESET_ANSI_CODE,self->sender_pid, self->state);
                    break;
            }
            break;
        default:
            printf("[MASTER_MSG_REPORT]  := type: %ld | sender: %d | state: %d \n", self->type, self->sender_pid, self->state);
    }
}
int master_msg_send(int id, struct master_msg_report * self,long type,long proc_type, pid_t sender_pid, short int state, Bool create, float budget){
    if (create == TRUE){master_msg_report_create(self,type,proc_type,sender_pid, state, budget);}
    while(msgsnd(id, self, sizeof(struct master_msg_report)-sizeof(long),0)<0){
        if (errno != EINTR) return -1;
    }
    return 0;
}
int master_msg_receive(int id, struct master_msg_report * self){
    if(msgrcv(id, self, sizeof(struct master_msg_report),0/*all incoming messages*/,IPC_NOWAIT)<0){
        if(errno == ENOMSG){
            return -2;
        }
        return -1;
    }
    return 0;
}
int master_msg_receive_info(int id, struct master_msg_report * self){
    if(msgrcv(id, self, sizeof(struct master_msg_report),INFO_BUDGET,0)<0){
        if(errno == ENOMSG){
            return -2;
        }
        return -1;
    }
    return 0;
}

int acknowledge(struct master_msg_report * self, ProcList list){
    long msg_type = self->type;
    int proc_type = self->proc_type;
    short int exec_state = self->state;
    Proc proc_to_update = get_proc_from_pid(list,self->sender_pid);
    if (proc_to_update == NULL){
        return -2;
    }
    switch(msg_type){
        case TERMINATION_END_CORRECTLY:
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
        case SIGNALS_OF_TERM_RECEIVED:
        case UNUSED_PROC:
        case INFO_BUDGET:
            if (self->budget>=0){
                proc_to_update->budget = self->budget;
            }
            proc_to_update->proc_state= exec_state;
            break;
        default:
            return -1;
    }
    return 0;
}
int check_msg_report(struct master_msg_report *msg_report, int msg_report_id_master, ProcList proc_list) {
    struct msqid_ds msg_rep_info;
    if (msgctl(msg_report_id_master, IPC_STAT, &msg_rep_info) < 0) {
        DEBUG_ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE MESSAGE QUEUE INFO");
        return -1;
    } else {
        /*fetching all msg if present*/
        if (msg_rep_info.msg_qnum != 0 &&
            msgrcv(msg_report_id_master, msg_report, sizeof(*msg_report) - sizeof(long), 0, 0)<0) {
            if (acknowledge(msg_report, proc_list)<0){
                ERROR_MESSAGE("IMPOSSIBLE TO MAKE THE ACKNOWLEDGE OF MASTER MESSAGE");
            }
        }
        return 0;
    }
}
