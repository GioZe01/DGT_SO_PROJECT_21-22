#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include "headers/user_msg_report.h"
#include "headers/simulation_errors.h"
#include "headers/glob.h"
#include "headers/boolean.h"
int user_msg_create(struct user_msg *self, long type, pid_t sender_pid, data *data) {
    switch (type) {
        case MSG_CONFIG_TYPE:
            self->data.users_id_to_pid = data->users_id_to_pid;
            break;
        case MSG_TRANSACTION_CONFIRMED_TYPE:
        case MSG_TRANSACTION_FAILED_TYPE:
            self->data.t = data->t;
            break;
        default:
            ERROR_MESSAGE("WRONG TYPE");
            return -1;
    }
    self->type = type;
    self->sender_pid = sender_pid;
    return 0;
}

void user_msg_print(struct user_msg *self) {
    switch (self->type) {
        case MSG_CONFIG_TYPE:
            printf("~ USER_MSG | type: CONF | sender: %d ~\n", self->sender_pid);
            break;
        case MSG_TRANSACTION_CONFIRMED_TYPE:
            printf("~ USER_MSG | type: TRANSACTION | %s%s status: CONFIRMED %s | sender_pid: %d | amount: %f "
                   "| t_sender: %d | t_receiver : %d ~\n",
                   COLOR_RESET_ANSI_CODE, COLOR_GREEN_ANSI_CODE, COLOR_RESET_ANSI_CODE, self->sender_pid,
                   self->data.t.amount, self->data.t.sender, self->data.t
                           .reciver);
            break;
        case MSG_TRANSACTION_FAILED_TYPE:
            printf("~ USER_MSG | type: TRANSACTION | %s%s status: CONFIRMED %s | sender_pid: %d | amount: %f "
                   "| t_sender: %d | t_receiver : %d ~\n",
                   COLOR_RESET_ANSI_CODE, COLOR_RED_ANSI_CODE, COLOR_RESET_ANSI_CODE, self->sender_pid,
                   self->data.t.amount, self->data.t.sender, self->data.t
                           .reciver);
            break;
    }
}
int user_msg_snd(int id, struct user_msg *msg,long type, data *data, pid_t sender, Bool create){
    if (create == TRUE){user_msg_create(&msg, type, sender, data);}
    printf("ID RECEIVED: %d\n", id);
    while(msgsnd(id, msg, sizeof (*msg)-sizeof (long), 0)<0){
        if (errno!=EINTR) return -1;
    }
    return 0;
}
int user_msg_receive(int id, struct user_msg * msg, long type){
    switch (type) {
        case MSG_CONFIG_TYPE:
            if(msgrcv(id, msg, sizeof(struct user_msg)- sizeof(long), type, 0)<0){
                return -1;
            }
            break;
        default:
            if(msgrcv(id, msg, sizeof(struct user_msg)- sizeof(long), type, IPC_NOWAIT)<0){
                if(errno == ENOMSG){
                    return -2;
                }
                return -1;
            }
            break;
    }
    return 0;
}