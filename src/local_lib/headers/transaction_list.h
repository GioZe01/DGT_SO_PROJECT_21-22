/**
 * \file transaction_list.h
 * \brief Header file for the transaction_list.c file.
 * Contains the definition of the Transaction structure and the typedef for the QUEUE. (ADT is implemented in transaction_list.c)
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
#define DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "boolean.h"
/**
 * \defgroup TRANSACTION STATES
 * \brief States of the transaction
 */
/** @{ */
#define TRANSACTION_SUCCES 0
#define TRANSACTION_FAILED -1
#define TRANSACTION_WAITING 1
/** @} */
/**
 * \struct Transaction
 * \brief Structure for the transaction.
 */
struct Transaction{
    short int t_type; /**< Type of the transaction. */
    struct timespec timestamp; /**< Timestamp of the transaction. */
    pid_t sender; /**< PID of the sender. */
    pid_t reciver; /**< PID of the reciver. */
    float amount; /**< Amount of the transaction. */
    float reward; /**< Reward of the transaction. */
    int hops; /**< Number of hops of the transaction. */
};

/**
 * \typedef Queue
 * \brief Typedef for the transactions queue.
 */
typedef struct transaction_list *Queue;
/*Functionality*/
int create_transaction(struct Transaction *t,pid_t sender, pid_t receiver, float amount);
/**
 * Initialize the queue;
 * @return the queue initialized
 */
Queue queue_create(void);
/**
 * Free the memory from the queue
 * @param q the queue to be destroyed
 */
void queue_destroy(Queue q);
/**
 * Insert a transaction in the given queue
 * @param q the queue to operate on
 * @param i transaction to be added
 */
void queue_append(Queue q, struct Transaction i);
/**
 * Remove the first Transaction in the queue
 * @param q the queue to operate on
 */
void queue_remove_head(Queue q);
/**
 * Get the first element of the queue
 * @param q the queue to operate on
 * @return The first Transaction of the queue
 */
struct Transaction * queue_head(Queue q);
/**
 * Get the last element of the queue
 * @param q
 * @return The last Transaction of the queue
 */
struct Transaction queue_last(Queue q);
/**
 * Check if the queue contains element
 * @param q the queue to operate on
 * @return TRUE if it's empty. FALSE otherwise
 */
Bool queue_is_empty(Queue q);
/**
 * Calculate the reward for each transaction with give perc and apt the amount
 * @param q the queue from whom is calculated the reward
 * @param percentage the percentage to calculate 4 each transaction
 * @return -1 in case of FAILURE. positive float otherwise
 */
float queue_apt_amount_reward(Queue q, float percentage);
/**
 * Return the total reward summing each transaction reward
 * @param q queue to make the operation on
 * @return -1 in case of FAILURE, the total reward amount otherwise
 */
float queue_get_reward(Queue q);
/**
 * Print the queue in a prettify kinda way
 * @param q the queue to be printed
 */
void queue_print(Queue q);
/**
 * Print the transaction given
 * @param t the transaction to be printed
 */
void transaction_print(struct Transaction t);
/**
 * Return the number of transactions in the linked list
 * @param q the list to get the num of transactions
 * @return the number of transaction. -1 in case of error.
 */
int get_num_transactions(Queue q);
/**
 * From the given queue load the data into a vector rather then a linked list
 * @param q the queue to load the data from
 * @param vector pointer of transactions to be loaded
 * @return -1 in case of failure. 0 otherwise
 */
int queue_to_array(Queue q, struct Transaction* vector);

/**
 * From the given vector load the data into the given queue
 * @param q the queue to load the data from
 * @param vector pointer of transactions to be loaded
 * @return -1 in case of failure. 0 otherwise
 */
int array_to_queue(Queue q, struct Transaction* vector);

/**
 * Copy n transactions from queue q to queue r and remove them from q
 * @param q the queue to copy from
 * @param r the queue to copy to
 * @param n the number of transactions to copy
 * @return FALSE in case of failure. TRUE otherwise
 */
Bool queue_copy_n_transactions(Queue q, Queue r, int n);

/**
 * Tells whether or not the given queue is empty or not
 * @param q the queue to perform the check on
 */
void empty_queue(Queue q);

/**
 * Create a empty transaction
 * @param t the transaction to be created
 */
void create_empty_transaction(struct  Transaction *t);

/**
 * Copy the given transaction into the given transaction
 * @param t the transaction to copy from
 * @param t_copy the transaction to copy into
 * @return -1 in case of failure 0 otherwise
 */
int copy_transaction(struct Transaction * t, struct Transaction* t_copy);
#endif /*DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H*/
