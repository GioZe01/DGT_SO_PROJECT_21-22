#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include "headers/node_msg_report.h"

node_msg_create(struct node_msg *self, long type, pid_t sender_pid, node_data *data) {
    switch (type) {
        case MSG_CONFIG_TYPE:
            self->data.nodes_id_to_pid = data->nodes_id_to_pid;
            break;
    }
    self->type = type;
    self->sender_pid = sender_pid;
    return 0;
}

void node_msg_print(struct node_msg *self) {
    switch (self->type) {
        case MSG_CONFIG_TYPE:
            printf("~ NODE_MSG | type: CONF | sender: %d ~\n", self->sender_pid);
            break;
        case MSG_NODE_ORIGIN_TYPE:
            printf("~ NODE_MSG | type: NODE ORIGIN | sender: %d ~\n", self->sender_pid);
            break;
    }
}

int node_msg_snd(int id, struct node_msg *msg, long type, node_data *data, pid_t sender, Bool create) {
    if (create == TRUE) { node_msg_create(msg, type, sender, data); }
    while (msgsnd(id, msg, sizeof(*msg) - sizeof(long), 0) < 0) {
        if (errno!= ENOMSG) {
            return -1;
        }
    }
    return 0;
}