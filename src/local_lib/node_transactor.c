#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
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

void node_create(struct node *self, pid_t node_pid, float budget, int tp_size, int block_size, Reward calc_reward) {
    self->pid = node_pid;
    self->budget = budget;
    self->tp_size = tp_size;
    self->block_size = block_size;
    self->transaction_block = queue_create();
    self->transaction_pool = queue_create();
    self->calc_reward = calc_reward;
}

void free_node(struct node *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE NODE OPERATIONS...");
    queue_destroy(self->transaction_pool);
    queue_destroy(self->transaction_block);
    DEBUG_NOTIFY_ACTIVITY_DONE("FREE NODE OPERATIONS DONE");
}

int update_budget(struct node *self) {
    float budget = 0;
    for (; queue_is_empty(self->transaction_block) == TRUE; queue_remove_head(self->transaction_block)) {
        budget += queue_head(self->transaction_pool).reward;
    }
    struct Transaction t_reward;
    t_reward.sender = -1;
    t_reward.reward = 0;
    return 0;
}

int add_to_pool(struct node *self, struct Transaction *t) {
    queue_append(self->transaction_pool, *t);
}

int add_to_block(struct node *self, struct Transaction *t) {
    queue_append(self->transaction_block, *t);
}
