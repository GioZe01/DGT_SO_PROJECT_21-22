#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>

/* Local*/
#include "headers/user_msg_report.h"
#include "headers/simulation_errors.h"
#include "headers/transaction_list.h"
#include "headers/glob.h"
#include "headers/boolean.h"

/**
 * \brief Return the corrispondent type of transaction based on the msg type
 * @param type The type of the message
 * @return The type of the transaction
 */
int get_transaction_type(int type);
int check_user_type(int type_given, int queue_id);

int user_msg_create(struct user_msg *self, long type, pid_t sender_pid, struct Transaction *t) {
    self->t = *t;
    self->type = type;
    self->sender_pid = sender_pid;
    return 0;
}

void user_msg_print(struct user_msg *self) {
    switch (self->type) {
        case MSG_TRANSACTION_CONFIRMED_TYPE:
            printf("~ USER_MSG | type: TRANSACTION | %s%s status: CONFIRMED %s | sender_pid: %d | amount: %f "
                   "| t_sender: %d | t_receiver : %d ~\n",
                   COLOR_RESET_ANSI_CODE, COLOR_GREEN_ANSI_CODE, COLOR_RESET_ANSI_CODE, self->sender_pid,
                   self->t.amount, self->t.sender, self->t
                           .reciver);
            break;
        case MSG_TRANSACTION_FAILED_TYPE:
            printf("~ USER_MSG | type: TRANSACTION | %s%s status: CONFIRMED %s | sender_pid: %d | amount: %f "
                   "| t_sender: %d | t_receiver : %d ~\n",
                   COLOR_RESET_ANSI_CODE, COLOR_RED_ANSI_CODE, COLOR_RESET_ANSI_CODE, self->sender_pid,
                   self->t.amount, self->t.sender, self->t
                           .reciver);
            break;
        default:
            ERROR_MESSAGE("TRANSACTION TYPE NOT RECONIZED");
            break;
    }
}

int user_msg_snd(int id, struct user_msg *msg, long type, struct Transaction *t, pid_t sender, Bool create, int queue_id) {
    t->t_type = get_transaction_type(type);
    if (create == TRUE) { user_msg_create(msg, check_user_type(type,queue_id), sender, t); }
    while (msgsnd(id, msg, sizeof(struct user_msg) - sizeof(long), 0) < 0) {
        if (errno != EINTR) return -1;
    }
    return 0;
}

int user_msg_receive(int id, struct user_msg *msg, long type) {
    if (msgrcv(id, msg, sizeof(struct user_msg) - sizeof(long), type, IPC_NOWAIT) < 0) {
        if (errno == ENOMSG) {
            return -2;
        }
        return -1;
    }
    printf("MESSAGE RECEIVED\n");
    return 0;
}
int check_user_type(int type_given, int queue_id){
    switch(type_given){
        case MSG_TRANSACTION_CONFIRMED_TYPE:
            return queue_id;
        default:
            return queue_id-type_given;
    }
}
int get_transaction_type(int type){
    switch(type){
        case MSG_TRANSACTION_INCOME_TYPE:
        case MSG_TRANSACTION_CONFIRMED_TYPE:
            return TRANSACTION_SUCCES;
        case MSG_TRANSACTION_FAILED_TYPE:
            return TRANSACTION_FAILED;
        default:
            return -1;
    }
}
