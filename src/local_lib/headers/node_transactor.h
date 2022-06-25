/**
 * \file node_transactor.h
 * \brief Header file for node_transactor.c
 * Contains the definition of the functions, structures and typedefs used to handle the node_transactor
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H
#define DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H


#include "transaction_list.h"

/**
 * \typedef Reward function type
 * \brief Type of the reward function used to calculate the reward for the current block of transactions in the node
 * \param [in] self current node
 * \param [in] percentage percentage of the reward
 * \param [in] use_default if true, the percentage is ignored and the default percentage is used
 * \param [in] block_tot_reward pointer in which is stored the total reward of the block
 */
typedef int(*Reward)(struct node *self, float percentage, Bool use_default, float *block_tot_reward);

/**
 * \struct node
 * \brief Node structure used to represent a node
 */
struct node {
    pid_t pid; /**< PID of the node */
    int node_id; /**< Index id of the node into the message queues for nodes*/
    short int exec_state;/**< Current state of the node proc*/
    float budget; /**< Current budget of the node */
    int tp_size; /**< Current size of the transaction pool */
    int block_size; /**< Current size of the block */
    float percentage; /**< Percentage of the budget to be used for the reward */
    Queue transactions_pool;/**< Queue of the transactions pool */
    Queue transactions_block; /**< Queue of the transactions block */
    Reward calc_reward; /**< Reward function used to calculate the reward for the node based on the transaction */
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
void
node_create(struct node *self, pid_t node_pid, int node_id, float budget, int tp_size, int block_size, float percentage,
            Reward calc_reward);

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
int calc_reward(struct node *self, float percentage, Bool use_default, float *block_tot_reward);

#endif /*DGT_SO_PROJECT_21_22_NODE_TRANSACTOR_H*/
