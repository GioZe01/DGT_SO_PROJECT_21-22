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
/* Struct definition*/
struct node {
    struct Transaction t;
    struct node *next;
};
/* ADT implementation*/
struct transaction_list {
    struct node *first;
    struct node *last;
    int transactions; /*number of transaction in the list*/
};
/* Local utility function*/
/**
 * Called when iterable action is called on empty queue to throw error
 */
static void queue_underflow(void);

/**
 * Transform the type given into a formatted char string
 * @param char_type the string in which u want the char string to be saved
 * @param t_type type of transaction to be transformed
 */
void get_status(char char_type[80], int t_type);

/**
 * Tells whether or not the given queue is empty or not
 * @param q the queue to perform the check on
 */
static void empty_queue(Queue q);

/**
 * Compare to transaction
 * @param t1 transaction 1
 * @param t2 transaction 2
 * @return TRUE if ==, FALSE otherwise
 */
Bool compare_transaction(struct Transaction t1, struct Transaction t2);

/**
 * Append the corrend node to the next one
 * @param where u want to append the node
 * @param to_append the node to append to the previous
 */
void append_to_node(struct node *where, struct node *to_append);

int create_transaction(struct Transaction *t, pid_t sender, pid_t receiver, float amount) {
    struct timespec timestamp;
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CREATING A TRANSACTION...");
    if (clock_gettime(CLOCK_REALTIME, &timestamp) < 0) {
        ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE CLOCK_TIME");
        return -1;
    }
    t->t_type = TRANSACTION_WAITING;
    t->hops = 0; /* User is not responsible for this value*/
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
        ERROR_MESSAGE("MALLOC FAILED IN THE CREATION OF QUEUE_T");
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
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_RUNNING("APPENDING TO TRANSACTION QUEUE A NEW TRANSACTION...");
#endif
    struct node *new_node;
    new_node = (struct node *) malloc(sizeof(struct node));
    if (new_node == NULL) {
        DEBUG_ERROR_MESSAGE("MALLOC ON NODE STRUCT IS NULL");
        ERROR_MESSAGE("Malloc failed in queue append");
        return;
    }
    new_node->t = t;
    new_node->next = NULL;
    if (queue_is_empty(q) == TRUE) /*adding the first ever node to the list*/
        q->first = q->last = new_node;
    else {/*is not the first node*/
        q->last->next = new_node;
        q->last = new_node;
    }
    q->transactions++;
#ifdef DEBUG_USER
    DEBUG_NOTIFY_ACTIVITY_DONE("APPENDING TO TRANSACTION QUEUE A NEW TRANSACTION DONE");
#endif
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
    } else{
        queue_underflow();
        return;
    }
}

int queue_remove(Queue q, struct Transaction t) {
    if (queue_is_empty(q) == TRUE)
        return -2;

    struct node *iterable = q->first;
    for (; compare_transaction(iterable->next->t, t) == TRUE; iterable = iterable->next);
    append_to_node(iterable, iterable->next->next);
    free(iterable->next);
    q->transactions--;
    return 0;
}

void append_to_node(struct node *where, struct node *to_append) {
    where->next = to_append;
}

Bool compare_transaction(struct Transaction t1, struct Transaction t2) {
    if (t1.reciver == t2.reciver &&
        t1.amount == t2.amount &&
        t1.sender == t2.sender &&
        t1.reward == t2.reward &&
        t1.hops == t2.hops &&
        t1.t_type == t2.t_type) {
        return TRUE;
    }
    return FALSE;
}

struct Transaction queue_head(Queue q) {
    if (queue_is_empty(q) == FALSE)
        return q->first->t;
    else {
#ifdef DEBUG_UNDERFLOW
        DEBUG_MESSAGE("queue_head in transaction: UNDERFLOW CALLED");
#endif
        queue_underflow();
    }
}

struct Transaction queue_last(Queue q) {
    if (queue_is_empty(q) == FALSE){
        return q->last->t;
    }
    else {
#ifdef DEBUG_UNDERFLOW
        DEBUG_MESSAGE("queue_head in transaction: UNDERFLOW CALLED");
#endif
        queue_underflow();
    }
}

Bool queue_is_empty(Queue q) {
    if (q->transactions == 0)
        return TRUE;
    return FALSE;
}

static void queue_underflow(void) {
    ERROR_MESSAGE("INVALID OPERATION ON AN EMPTY QUEUE");
    return;
}

float queue_apt_amount_reward(Queue q, float percentage) {
    if (queue_is_empty(q) == TRUE) {
        ERROR_MESSAGE("CALL APT AMOUNT-REWARD ON EMPTY QUEUE");
        return -1;
    }
    struct node *first = q->first;
    float total_reward = 0;

    for (; first != NULL; first = first->next) {
        first->t.reward = (first->t.amount * (percentage))/ 100;
        if (first->t.reward < 0) {
            ERROR_MESSAGE("NEGATIVE REWARD, CHECK VALUES");
            return -1;
        }
        first->t.amount -= first->t.reward;
        total_reward += first->t.reward;
    }
    return total_reward;
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
    return tot;
}

void queue_print(Queue q) {
    if (queue_is_empty(q) == TRUE) {
        ERROR_MESSAGE("PRINTING AN EMPTY QUEUE");
    }
    struct node *iterable;
    iterable = q->first;
    printf("--------- TRANSACTION LIST %d ---------\n", getpid());
    printf("| Current # of transactions: %d\n", q->transactions);
    printf("|--------------------------------\n");
    for (; iterable != NULL; iterable = iterable->next) {
        transaction_print(iterable->t);
    }
    printf("----------------TRANSACTION LIST END--------------------\n");
#ifdef DEBUG_NODE_TP
    sleep(0.1); /* I know about nanosleep but sleep is fuster to write and np at this stage for debug*/
#endif
}

void get_status(char char_type[80], int t_type) {
    strcpy(char_type, COLOR_RESET_ANSI_CODE);
    switch (t_type) {
        case TRANSACTION_WAITING:
            strcat(char_type, COLOR_YELLOW_ANSI_CODE);
            strcat(char_type, "WAITING");
            break;
        case TRANSACTION_FAILED:
            strcat(char_type, COLOR_RED_ANSI_CODE);
            strcat(char_type, "FAILED");
            break;
        case TRANSACTION_SUCCES:
            strcat(char_type, COLOR_GREEN_ANSI_CODE);
            strcat(char_type, "SUCCESS");
            break;
        default:
            strcpy(char_type, "NO INFO");
    }
    strcat(char_type, COLOR_RESET_ANSI_CODE);
}

void transaction_print(struct Transaction t) {
    char char_type[80];
    get_status(char_type, t.t_type);
    printf("| status: %s | sender: %d | receiver: %d | amount: %f | hops: %d | reward: %f | timestamp: %ld,%ld \n",
           char_type, t.sender, t.reciver, t.amount,
           t.hops, t.reward, t.timestamp.tv_sec, t.timestamp.tv_nsec);
}

int get_num_transactions(Queue q) {
    if (queue_is_empty(q) == FALSE)
        return q->transactions;
    return 0;
}

int queue_to_array(Queue q, struct Transaction vector[]) {
    if (vector != NULL || get_num_transactions(q)==0) {
        struct node *iterable = q->first;
        int i = 0;
        for (; iterable != NULL; iterable = iterable->next) {
            vector[i] = iterable->t;
            i++;
        }
        return 0;
    }
    return -1;
}
int array_to_queue(Queue q, struct Transaction* vector){
    if (vector == NULL){
        return -1;
    }
    int i = 0;
    for (;i<SO_BLOCK_SIZE; i++){
        queue_append(q, vector[i]);
    }
    return 0;
}

Bool queue_copy_n_transactions(Queue q, Queue r, int n){
    if (n > q->transactions) {
        return FALSE;
    }
    int to_remove= n;
    struct node *iterable = q->first;
    for (; iterable != NULL && n > 0; iterable = iterable->next) {
        queue_append(r, iterable->t);
        n--;
    }
    /*remove the transactions from q*/
    for (; to_remove > 0; to_remove--) {
        queue_remove_head(q);
    }
    return TRUE;
}
