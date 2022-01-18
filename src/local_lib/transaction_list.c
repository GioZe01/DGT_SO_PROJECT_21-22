/* Sys import*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
/*Local import */
#include "headers/transaction_list.h"
#include "headers/simulation_errors.h"
#include "headers/glob.h"
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

char *get_status(int t_type);

static void empty_queue(Queue q);

struct node {
    struct Transaction t;
    struct node *next;
};
/*Implement the real queue structure*/
struct transaction_list {
    struct node *first;
    struct node *last;
    int transactions; /*number of transaction in the list*/
};

int create_transaction(struct Transaction *t, pid_t sender, pid_t receiver, float amount) {

    struct timespec timestamp;
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING A TRANSACTION...");
    if (clock_gettime(CLOCK_REALTIME, &timestamp) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE CLOCK_TIME");
        return -1;
    }
    t->t_type = TRANSACTION_WAITING;
    t->hops = 0;
    t->sender = sender;
    t->reciver = receiver;
    t->timestamp = timestamp;
    t->amount = amount;
    t->reward = 0; /* Is not responsible*/
    DEBUG_NOTIFY_ACTIVITY_DONE("CREATING A TRANSACTION DONE");
    return 0;
}

Queue queue_create() {
    Queue q = malloc(sizeof(struct transaction_list));
    if (q == NULL) {
        ERROR_MESSAGE("Malloc failed in the creation of quque_t");
        return NULL;
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

void queue_append(Queue q, struct Transaction t) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("APPENDING TO TRANSACTION QUEUE A NEW TRANSACTION...");
    struct node *new_node;
    if ((new_node = malloc(sizeof(struct node)) == NULL)) {
        DEBUG_ERROR_MESSAGE("MALLOC ON NODE STRUCT IS NULL");
        ERROR_MESSAGE("Malloc failed in queue append");
        return;
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

struct Transaction queue_head(Queue q) {
    if (queue_is_empty(q) == FALSE)
        return q->first->t;
    else {
        DEBUG_MESSAGE("queue_head in transaction: UNDERFLOW CALLED");
        queue_underflow();
    }
}

struct Transaction queue_last(Queue q) {
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
    return;
}

int queue_apt_amount_reward(Queue q, int percentage) {
    if (queue_is_empty(q) == TRUE) {
        ERROR_MESSAGE("CALL APT AMOUNT-REWARD ON EMPTY QUEUE");
        return -1;
    }
    struct node *first = q->first;

    for (; first != NULL; first = first->next) {
        first->t.reward = (float) (first->t.amount * ((float) percentage)) / 100;
        if (first->t.reward < 0) {
            ERROR_MESSAGE("NEGATIVE REWARD, CHECK VALUES");
            return -1;
        }
        first->t.amount -= first->t.reward;
    }
    return 0;
}

float queue_get_reward(Queue q) {
    if (queue_is_empty(q) == TRUE) {
        ERROR_MESSAGE("CALL GET REWARD ON EMPTY QUEUE");
        return -1;
    }
    float tot = 0;
    struct node *first = q->first;

    for (; first != NULL; first = first->next) {
        tot += first->t.reward;
    }
    return 0;
}

void queue_print(Queue q) {
    if (queue_is_empty(q) == TRUE) {
        ERROR_MESSAGE("PRINTING AN EMPTY QUEUE");
    }
    struct node *iterable = q->first;
    printf("--------- TRANSACTION LIST ---------\n");
    printf("| Current # of transactions: %d", q->transactions);
    printf("|--------------------------------");
    for (; iterable != NULL; iterable = iterable->next) {
        printf("| status: %s | sender: %d | receiver: %d | amount: %d | hops: %d | reward: %d | timestamp: %ld,%ld \n",
               get_status(iterable->t.t_type), iterable->t.sender, iterable->t.reciver, iterable->t.amount,
               iterable->t.hops, iterable->t.reward, iterable->t.timestamp.tv_nsec, iterable->t.timestamp.tv_nsec);
    }
    printf("------------------------------------\n");
}

char *get_status(int t_type) {
    char *char_type;
    switch (t_type) {
        case TRANSACTION_WAITING:
            char_type = "WAITING";
            strcat(strcat(strcat(COLOR_RESET_ANSI_CODE, COLOR_YELLOW_ANSI_CODE), char_type), COLOR_RESET_ANSI_CODE);
            break;
        case TRANSACTION_FAILED:
            char_type = "FAILED";
            strcat(strcat(strcat(COLOR_RESET_ANSI_CODE, COLOR_RED_ANSI_CODE), char_type), COLOR_RESET_ANSI_CODE);
            break;
        case TRANSACTION_SUCCES:
            char_type = "SUCCESS";
            strcat(strcat(strcat(COLOR_RESET_ANSI_CODE, COLOR_GREEN_ANSI_CODE), char_type), COLOR_RESET_ANSI_CODE);
            break;
        default:
            char_type = "NO INFO";
    }
    return char_type;
}