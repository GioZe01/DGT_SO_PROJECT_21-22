#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include "headers/node_msg_report.h"
#include "headers/simulation_errors.h"

int node_msg_create(struct node_msg *self, long type, pid_t sender_pid, struct Transaction *t) {
    switch (type) {
        case MSG_NODE_ORIGIN_TYPE:
        case MSG_TRANSACTION_TYPE:
            self->t = *t;
            break;
        default:
            ERROR_MESSAGE("WRONG TYPE");
            return -1;
    }
    self->type = type;
    self->sender_pid = sender_pid;
    return 0;
}

void node_msg_print(struct node_msg *self) {
    switch (self->type) {
        case MSG_NODE_ORIGIN_TYPE:
            printf("~ NODE_MSG | type: NODE ORIGIN | sender: %d ~\n", self->sender_pid);
            break;
        case MSG_TRANSACTION_TYPE:
            printf("~ NODE_MSG | type: TRANSACTION TYPE | sender: %d ~\n", self->sender_pid);
            break;
    }
}

int node_msg_snd(int id, struct node_msg *msg, long type, struct Transaction *t, pid_t sender, Bool create) {
    if (create == TRUE) { node_msg_create(msg, type, sender, t); }
    while (msgsnd(id, msg, sizeof(*msg) - sizeof(long), 0) < 0) {
        if (errno != ENOMSG) {
            return -1;
        }
    }
    return 0;
}

int node_msg_receive(int id, struct node_msg *msg, long type) {
    if (msgrcv(id, msg, sizeof(struct node_msg) - sizeof(type), type, IPC_NOWAIT) < 0) {
        if (errno == ENOMSG) {
            return -2;
        }
        return -1;

    }
    return 0;
}