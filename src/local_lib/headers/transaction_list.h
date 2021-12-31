
#ifndef DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
#define DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H
/*TODO: Implementare transaction -> ogni user avra una transaction list per quelli processati e quelli da fare*/
#include "boolean.h"
#include "user.h"
typedef struct {
    int timestamp;
    User sender;
    User reciver;
    float amount; /*TODO: verificare se basta*/
    float reward;
}Transaction;
typedef struct transaction_list *Queue;/*Real impl in transaction_list.c*/
Queue queue_create(void);
void queue_destroy(Queue q);
void queue_append(Queue, Transaction i);
void queue_remove_head(Queue q);
Transaction queue_head(Queue q);
Transaction queue_last(Queue q);
Bool queue_is_empty(Queue q);
#endif /*DGT_SO_PROJECT_21_22_TRANSACTION_LIST_H*/
