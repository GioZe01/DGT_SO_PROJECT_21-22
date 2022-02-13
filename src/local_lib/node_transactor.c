#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include "headers/simulation_errors.h"
#include "headers/transaction_list.h"
#include "headers/node_transactor.h"
#include "headers/boolean.h"

#ifdef DEBUG

#include "headers/debug_utility.h"

#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

void node_proc_block_create(struct node*self, pid_t node_pid, float budget, int block_size,int percentage, Reward calc_reward) {
    node_block block;
    block.budget = budget;
    block.block_size = block_size;
    block.calc_reward = calc_reward;
    block.percentage = percentage;
    self->transactions_list= queue_create();
    self->pid = node_pid;
    self->type.block = block;
}
void node_proc_tp_create(struct node*self, pid_t node_pid, int tp_size){
    node_tp tp_node;
    tp_node.tp_size = tp_size;
    self->pid = node_pid;
    self->transactions_list= queue_create();
    self->type.tp= tp_node;
}

void free_node(struct node *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE NODE OPERATIONS...");
    queue_destroy(self->transactions_list);
    DEBUG_NOTIFY_ACTIVITY_DONE("FREE NODE OPERATIONS DONE");
}

int update_budget(struct node *self) {
    float budget = 0;
    struct Transaction t_reward;
    struct timespec timestamp;

    if (clock_gettime(CLOCK_REALTIME, &timestamp) < 0 ) {
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE CLOCK_TIME");
        return -1;
    }
    t_reward.sender = -1;
    t_reward.reward = 0;
    t_reward.amount = queue_get_reward(self->transactions_list);
    t_reward.timestamp = timestamp;
    self->type.block.budget+=t_reward.amount;
    return 0;
}

int add_to_transactions_list(struct node *self, struct Transaction *t) {
    queue_append(self->transactions_list, *t);
}

int calc_reward(struct node* self, int percentage, Bool use_default){
    switch (use_default) {
        case TRUE:
            if(queue_apt_amount_reward(self->transactions_list, self->type.block.percentage)<0)return -1;
            break;
        case FALSE:
            if(queue_apt_amount_reward(self->transactions_list, percentage)<0)return -1;
            break;
        default:
            ERROR_MESSAGE("MISSING PARAMETER USE DEFAULT");
            return -1;
    }
    return 0;
}
