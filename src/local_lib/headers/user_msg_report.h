#define _GNU_SOURCE
#ifndef DGT_SO_PROJECT_21_22_USER_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_USER_MSG_REPORT_H
#include <unistd.h>
#include "transaction_list.h"
#define MSG_CONFIG_TYPE 1
#define MSG_TRANSACTION_FAILED_TYPE 2
#define MSG_TRANSACTION_CONFIRMED_TYPE 3
struct user_msg {
    long type;
    pid_t sender_pid;
    union{
        struct Transaction t;
        struct {
            int * users_id_to_pid;
        }users_aknowledge;
    }data;

};
void user_msg_create(struct user_msg* self, long type, pid_t sender_pid, union data);
void user_msg_print(struct user_msg* self);
#endif /*DGT_SO_PROJECT_21_22_USER_MSG_REPORT_H*/
