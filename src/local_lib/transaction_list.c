#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
/*Local import */
#include "headers/transaction_list.h"
#include "headers/simulation_errors.h"
#include "headers/glob_vars.h"
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
        EXIT_PROCEDURE_USER(EXIT_FAILURE);
    }
    q->first = NULL;
    q->last = NULL;
    q->transactions = 0;
    return q;
}

void queue_destroy(Queue q) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("REMOVING ITEM FROM TRANSACTION QUEUE... ");
    empty_queue(q);
    free(q);
    DEBUG_NOTIFY_ACTIVITY_DONE("REMOVING ITEM FROM TRANSACTION QUEUE COMPLETED ");
}

static void empty_queue(Queue q) {
    while (queue_is_empty(q) == FALSE) {
        queue_remove_head(q);
    }
}

void queue_append(Queue q, Transaction t) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("APPENDING TO TRANSACTION QUEUE A NEW TRANSACTION...");
    struct node *new_node;
    if ((new_node = malloc(sizeof(struct node)) == NULL)) {
        DEBUG_ERROR_MESSAGE("MALLOC ON NODE STRUCT IS NULL");
        ERROR_MESSAGE("Malloc failed in queue append");
        EXIT_PROCEDURE_USER(EXIT_FAILURE);
    }
    new_node->t = t;
    new_node->next = NULL;
    if (queue_is_empty(q) == TRUE) /*adding the first ever node to the list*/
        q->first = q->last = new_node;
    else /*is not the first node*/
        q->last->next = new_node;
    q->last = new_node;
    q->transactions++;

    DEBUG_NOTIFY_ACTIVITY_RUNNING("APPENDING TO TRANSACTION QUEUE A NEW TRANSACTION DONE");
}

void queue_remove_head(Queue q) {
    if (queue_is_empty(q) == FALSE) {
        struct node *temp = q->first;
        if (q->first == q->last)
            q->first = q->last = NULL;
        else
            q->first = q->first->next;
        free(temp);
        q->transactions--;
    } else
        queue_underflow();
}

Transaction queue_head(Queue q) {
    if (queue_is_empty(q) == FALSE)
        return q->first->t;
    else {
        DEBUG_MESSAGE("queue_head in transaction: UNDERFLOW CALLED");
        queue_underflow();
    }
}

Transaction queue_last(Queue q) {
    if (queue_is_empty(q) == FALSE)
        return q->last->t;
    else {
        DEBUG_MESSAGE("queue_last in transaction: UNDERFLOW CALLED");
        queue_underflow();
    }
}

Bool queue_is_empty(Queue q) {
    if (q->transactions == 0)
        return TRUE;
    return FALSE;
}

static void queue_underflow(void) {
    DEBUG_ERROR_MESSAGE("queue_underflow has been called");
    ERROR_MESSAGE("Invalid Operation on Queue empty");
    EXIT_PROCEDURE_USER(EXIT_FAILURE);
}