#include <stdio.h>
#include <sys/types.h>
#include "headers/master_msg_report.h"
#include "headers/glob.h"

void master_msg_report_create(struct master_msg_report *self, long type, pid_t sender_pid, short int state) {
    self->type = type;
    self->sender_pid = sender_pid;
    self->state = state;
}

void master_msg_report_print(struct master_msg_report *self) {
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
