#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include "headers/node_msg_report.h"
#include "headers/simulation_errors.h"
#include "headers/transaction_list.h"
#include "headers/boolean.h"

int check_default(long type, int queue_id);

int node_msg_create(struct node_msg *self, long type, pid_t sender_pid, struct Transaction *t, int message_type) {
    if (message_type == MSG_NODE_ORIGIN_TYPE) {
        t->hops++;
    }
    if (message_type != MSG_MASTER_ORIGIN_ID) {
        copy_transaction(t, &self->t);
    } else {
        create_empty_transaction(&self->t);
        self->t.sender = t->sender;
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
            printf("~ NODE_MSG | type: TRANSACTION TYPE | sender: %d | type: %ld ~\n", self->sender_pid, self->type);
            break;
        case MSG_MASTER_ORIGIN_ID:
            printf("~ NODE_MSG | type: MASTER ORIGIN ID | sender: %d~\n", self->sender_pid);
            break;
        default:
            ERROR_MESSAGE("NODE MESSAGE TYPE NOT RECONIZED");
            break;
    }
}

int
node_msg_snd(int id, struct node_msg *msg, long type, struct Transaction *t, pid_t sender, Bool create, int so_retry,
             int queue_id) {
    int retry = 0;
    if (create == TRUE) {
        node_msg_create(msg, check_default(type, queue_id), sender, t, type);
    }
    /** Check if there is space in the node message queue **/
    struct msqid_ds msq_ds;

    if (msgctl(id, IPC_STAT, &msq_ds) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO GET INFO ON NODE MESSAGE QUEUE");
    }
    printf("[%d] My bytes: %lu\n", getpid(), (msq_ds.msg_qnum + 1) * sizeof(struct node_msg));
    if ((msq_ds.msg_qnum + 1) * sizeof(struct node_msg) > msq_ds.msg_qbytes) {
        return -2;
    }
    while (msgsnd(id, msg, sizeof(struct node_msg) - sizeof(long), 0) < 0 && retry <= so_retry) {
        if (errno != EINTR) return -1;
        retry++;
    }
    if (retry == so_retry) {
        return -1;
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

int check_default(long type, int queue_id) {
    switch (type) {
        case MSG_MASTER_ORIGIN_ID:
            return queue_id - 2;
        case MSG_NODE_ORIGIN_TYPE:
            return queue_id - 1;
        case MSG_TRANSACTION_TYPE:
            return queue_id;
        default:
            ERROR_MESSAGE("WRONG TYPE");
            return -1;
    }
}
