#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "transaction_list.h"
#include "simulation_errors.h"
#include "glob_vars.h"
#include "boolean.h"

static void queue_underflow(void);

static void empty_queue(Queue q);

struct node {
    Transaction t;
    struct node *next;
};
/*Implement the real queue structure*/
struct transaction_list {
    struct node *first;
    struct node *last;
    int transactions; /*number of transaction in the list*/
};

Queue queue_create() {
    Queue q = malloc(sizeof(struct transaction_list));
    if (q == NULL) {
        ERROR_MESSAGE("Malloc failed in the creation of quque_t");
        EXIT_PROCEDURE(EXIT_FAILURE);
    }
}

void queue_destroy(Queue q) {
    empty_queue(q);
    free(q);
}

static void empty_queue(Queue q) {
    while (queue_is_empty(q) == FALSE) {
        queue_remove_head(q);
    }
}

void queue_append(Queue q, Transaction t) {
    struct node *new_node;
    if ((new_node = malloc(sizeof(struct node)) == NULL)) {
        ERROR_MESSAGE("Malloc failed in queue append");
        EXIT_PROCEDURE(EXIT_FAILURE);
    }
    new_node->t = t;
    new_node->next = NULL;
    if (queue_is_empty(q) == TRUE) /*adding the first ever node to the list*/
        q->first = q->last = new_node;
    else /*is not the first node*/
        q->last->next = new_node;
    q->last = new_node;
    q->transactions++;
}

void queue_remove_head(Queue q) {
    if (queue_is_empty(q) == FALSE) {
        struct node *temp = q->first;
        if(q-> first == q->last)
            q->first = q->last = NULL;
        else
            q->first = q->first->next;
        free(temp);
        q->transactions--;
    } else
        queue_underflow();
}
Transaction queue_head(Queue q){
    if(queue_is_empty(q) == FALSE)
        return q->first->t;
    else
        queue_underflow();
}
Transaction queue_last(Queue q){
    if(queue_is_empty(q) == FALSE)
        return q->last->t;
    else
        queue_underflow();
}
Bool queue_is_empty(Queue q){
   if(q->transactions == 0)
       return TRUE;
   return FALSE;
}
static void queue_underflow(void) {
    ERROR_MESSAGE("Invalid Operation on Queue empty");
    EXIT_PROCEDURE(EXIT_FAILURE);
}