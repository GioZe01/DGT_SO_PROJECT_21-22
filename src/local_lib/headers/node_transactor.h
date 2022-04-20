#ifndef DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H
#define DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H


#include "transaction_list.h"

typedef int(*Reward)(struct node *self, float percentage, Bool use_default, float * block_tot_reward);
struct node {
    pid_t pid;
    int node_id; /* Index id of the node into the message queues for nodes*/
    short int exec_state;/* Current state of the node proc*/
    float budget;
    int tp_size;
    int block_size;
    float percentage;
    Queue transactions_pool;/*Make it a pointer to shm*/
    Queue transactions_block;
    Reward calc_reward;
};

/**
 * Initialize the node with the given param, and the transaction list as empty
 * @param self node to be initialized
 * @param node_pid pid of the node proc associated with this struct
 * @param node_id id of the node
 * @param budget initial budget of the node
 * @param tp_size size of the transaction pool
 * @param block_size size of the transaction block
 * @param percentage percentage of the budget to be used for the reward
 * @param calc_reward pointer to function that calculate the reward
 */
void node_create(struct node *self, pid_t node_pid, int node_id, float budget, int tp_size, int block_size, float percentage,Reward calc_reward);

/**
 * Free the memory from the queue list of transactions
 * @param self
 */
void free_node(struct node *self);

/**
 * Update the budget of the node based on the processed nodes;
 * @param self node to be updated
 * @return  -1 in case of FAILURE. 0 otherwise
 */
int update_budget(struct node *self);

/**
 * Calculate the expected reward on the transaction block of the given node and update current node
 * budget
 * @param self node to operate the calc on
 * @param percentage the non default percentage that u want to use to calc the reward
 * @param use_default if u want to use the default percentage loaded in the node on creation time
 * @param block_tot_reward the total block reward value
 * @return -1 in case of FAILURE. 0 otherwise
 */
int calc_reward(struct node* self, float percentage, Bool use_default, float * block_tot_reward);
#endif /*DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H*/
