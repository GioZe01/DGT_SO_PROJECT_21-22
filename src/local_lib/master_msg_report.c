#define _GNU_SOURCE
/* Sys */
#include <stdio.h>
#include <sys/msg.h>
#include <errno.h>
/* Local */
#include "headers/master_msg_report.h"
#include "headers/glob.h"

void master_msg_report_create(struct master_msg_report *self, long type,long proc_type, pid_t sender_pid, short int state) {
    self->type = type;
    self->proc_type = proc_type;
    self->sender_pid = sender_pid;
    self->state = state;
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
int master_msg_send(int id, struct master_msg_report * self,long type,long proc_type, pid_t sender_pid, short int state, Bool create){
    if (create == TRUE){master_msg_report_create(self,type,proc_type,sender_pid, state);}
    while(msgsnd(id, self, sizeof(*self)-sizeof(long),0)<0){
        if (errno != ENOMSG) return -1;
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
