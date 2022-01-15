
#ifndef DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H
#define DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H


#include "transaction_list.h"

typedef float(*Reward)(struct node *self);

struct node {
    pid_t pid;
    float budget;
    int tp_size;
    int block_size;
    int percentage;
    Queue transaction_pool;
    Queue transaction_block;
    Reward calc_reward;
};

/**
 * Initialize the node with the given param, and the transaction list as empty
 * @param self node to be initialized
 * @param node_pid pid of the node proc associated with this struct
 * @param budget initial budget of the node
 * @param tp_size size of the transaction pool
 * @param block_size size of the transaction block
 * @param calc_reward pointer to function that calculate the reward
 */
void node_create(struct node *self, pid_t node_pid, float budget, int tp_size, int block_size, int percentage,Reward calc_reward);

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
 * Add to the transaction pull the given transaction
 * @param self at which u want the transaction to be added
 * @param t transaction to be added
 * @return -1 in case of FAILURE. 0 otherwise
 */
int add_to_pool(struct node *self, struct Transaction *t);

/**
 * Add to the transaction block the given transaction
 * @param self at wich u want the transaction to be added
 * @param t transaction to be added
 * @return -1 in case of FAILURE. 0 otherwise
 */
int add_to_block(struct node *self, struct Transaction *t);
/**
 * Calculate the expected reward on the transaction block of the given node
 * @param self node to operate the calc on
 * @param percentage the non default percentage that u want to use to calc the reward
 * @param use_default if u want to use the default percentage loaded in the node on creation time
 * @return -1 in case of FAILURE. 0 otherwise
 */
int calc_reward(struct node * self, int percentage, Bool use_default);
#endif /*DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H*/
