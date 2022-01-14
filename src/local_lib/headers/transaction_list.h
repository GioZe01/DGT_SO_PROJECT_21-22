#define _GNU_SOURCE
#ifndef DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
#define DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
/*TODO: Implementare transaction -> ogni user_transaction avra una transaction list per quelli processati e quelli da fare*/
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "boolean.h"
#define TRANSACTION_SUCCES 0
#define TRANSACTION_FAILED -1
#define TRANSACTION_WAITING 1
struct Transaction{
    short int t_type;
    struct timespec timestamp;
    pid_t sender;
    pid_t reciver;
    float amount; /*Cambiare nel caso in cui non bastasse*/
    float reward;
    int hops;
};
typedef struct transaction_list *Queue;/*Real impl in transaction_list.c*/
/*Functionality*/
struct Transaction create_transaction(short int t_type, struct timespec timestamp, pid_t sender, pid_t receiver, );
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
 * @param q the queue to ooperate on
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
struct Transaction queue_head(Queue q);
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
 * @param q
 * @param percentage
 * @return -1 in case of FAILURE. 0 otherwise
 */
int queue_apt_amount_reward(Queue q, int percentage);
/**
 * Return the total reward summing each transaction reward
 * @param q queue to make the operation on
 * @return -1 in case of FAILURE, the total reward amount otherwise
 */
float queue_get_reward(Queue q);



#endif /*DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H*/
