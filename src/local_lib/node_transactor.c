/* Sys */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
/* Local */
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

void node_create(struct node*self, pid_t node_pid,int node_id,float budget, int block_size,int percentage,int tp_size, Reward calc_reward){
   self->budget = budget;
    self->block_size = block_size;
    self->calc_reward = calc_reward;
    self->percentage = percentage;
    self->transactions_pool= queue_create();
    self->transactions_block= queue_create();
    self->pid = node_pid;
    self->node_id = node_id;
    self->tp_size = tp_size;
}

void free_node(struct node *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE NODE OPERATIONS...");
    queue_destroy(self->transactions_block);
    queue_destroy(self->transactions_pool);
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
    t_reward.amount = queue_get_reward(self->transactions_block);
    t_reward.timestamp = timestamp;
    self->budget+=t_reward.amount;
    return 0;
}


int calc_reward(struct node* self, int percentage, Bool use_default, float * block_tot_reward){
    switch (use_default) {
        case TRUE:
            *block_tot_reward = queue_apt_amount_reward(self->transactions_block, self->percentage);
            break;
        case FALSE:
            *block_tot_reward = queue_apt_amount_reward(self->transactions_block, percentage);
            break;
        default:
            ERROR_MESSAGE("MISSING PARAMETER USE DEFAULT");
            return -1;
    }
    if(block_tot_reward<0)return -1;
    return 0;
}
