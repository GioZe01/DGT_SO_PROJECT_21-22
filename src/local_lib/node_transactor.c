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

void node_create(struct node *self, pid_t node_pid, float budget, int tp_size, int block_size,int percentage, Reward calc_reward) {
    self->pid = node_pid;
    self->budget = budget;
    self->tp_size = tp_size;
    self->block_size = block_size;
    self->transaction_block = queue_create();
    self->transaction_pool = queue_create();
    self->calc_reward = calc_reward;
    self->percentage = percentage;
}

void free_node(struct node *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE NODE OPERATIONS...");
    queue_destroy(self->transaction_pool);
    queue_destroy(self->transaction_block);
    DEBUG_NOTIFY_ACTIVITY_DONE("FREE NODE OPERATIONS DONE");
}

int update_budget(struct node *self) {
    float budget = 0;
    struct Transaction t_reward;
    t_reward.sender = -1;
    t_reward.reward = 0;
    t_reward.amount = queue_get_reward(self->transaction_block);
    t_reward.timestamp =
    self->budget+=t_reward.amount;
    return 0;
}

int add_to_pool(struct node *self, struct Transaction *t) {
    queue_append(self->transaction_pool, *t);
}

int add_to_block(struct node *self, struct Transaction *t) {
    queue_append(self->transaction_block, *t);
}
int calc_reward(struct node* self, int percentage, Bool use_default){
    switch (use_default) {
        case TRUE:
            if(queue_apt_amount_reward(self->transaction_block, self->percentage)<0)return -1;
            break;
        case FALSE:
            if(queue_apt_amount_reward(self->transaction_block, percentage)<0)return -1;
            break;
        default:
            ERROR_MESSAGE("MISSING PARAMETER USE DEFAULT");
            return -1;
    }
    return 0;
}