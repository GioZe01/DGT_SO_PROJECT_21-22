
#ifndef DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
#define DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
/*TODO: Implementare transaction -> ogni user_transaction avra una transaction list per quelli processati e quelli da fare*/

#include <unistd.h>
#include <sys/types.h>
#include "boolean.h"
#define TRANSACTION_SUCCES 0
#define TRANSACTION_FAILED -1
#define TRANSACTION_WAITING 1
struct Transaction{
    short int t_type;
    double timestamp;
    pid_t sender;
    pid_t reciver;
    float amount; /*Cambiare nel caso in cui non bastasse*/
    float reward;
    int hops;
};
typedef struct transaction_list *Queue;/*Real impl in transaction_list.c*/
/*Functionality*/
Queue queue_create(void);
void queue_destroy(Queue q);
void queue_append(Queue q, struct Transaction i);
void queue_remove_head(Queue q);
struct Transaction queue_head(Queue q);
struct Transaction queue_last(Queue q);
Bool queue_is_empty(Queue q);
#endif /*DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H*/
