
#ifndef DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
#define DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
/*TODO: Implementare transaction -> ogni user_transaction avra una transaction list per quelli processati e quelli da fare*/

#include "boolean.h"
#define TRANSACTION_SUCCE 1
#define TRANSACTION_FAILED 0
#include <unistd.h>
#include <sys/types.h>

typedef struct {
    short int t_type;
    int timestamp;
    pid_t sender;
    pid_t reciver;
    float amount; /*TODO: verificare se basta*/
    float reward;
}Transaction;
typedef struct transaction_list *Queue;/*Real impl in transaction_list.c*/
/*Functionality*/
Queue queue_create(void);
void queue_destroy(Queue q);
void queue_append(Queue q, Transaction i);
void queue_remove_head(Queue q);
Transaction queue_head(Queue q);
Transaction queue_last(Queue q);
Bool queue_is_empty(Queue q);
#endif /*DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H*/
