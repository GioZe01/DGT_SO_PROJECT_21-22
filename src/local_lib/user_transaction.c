#define _GNU_SOURCE
/*  Standard    */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
/*  Local   */
#include "headers/user_transaction.h"

#ifdef DEBUG

#include "headers/debug_utility.h"

#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

pid_t extract_node(int nodes_num);

/**
 * Select a user pid from the vector pointer users_num randomly
 * @param users_num the pointer to the vector of users_id_to_pid
 * @return the pid_t of the user selected randomly
 */
pid_t extract_user(int *users_num);

float gen_amount(struct user_transaction *user);

void user_create(struct user_transaction *self, float budget, int pid, Balance balance, CalcCashFlow update_cash_flow) {
    self->pid = pid;
    self->budget = budget;
    self->u_balance = balance;
    self->transactions_done = queue_create();
    self->in_process = queue_create();
    self->cash_flow.entries = budget;
    self->cash_flow.outcomes = 0;
    self->expected_out = 0;
    self->update_cash_flow = update_cash_flow;
}

void free_user(struct user_transaction *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE USER OPERATION...");
    queue_destroy(self->in_process);
    queue_destroy(self->transactions_done);
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE USER OPERATION DONE");
}

Bool check_balance(struct user_transaction *self) {
    return self->u_balance >= 2 ? TRUE : FALSE;
}

float calc_balance(struct user_transaction *self) {
    printf("Current user entries: %f \n", self->budget);
    return fabs(self->cash_flow.entries) - fabs(self->cash_flow.outcomes) - fabs(self->expected_out);
}

int update_cash_flow(struct user_transaction *self, struct Transaction *t) {
    if (self->pid == t->reciver && t->t_type == TRANSACTION_SUCCES) { /*Getting Reacher*/
        self->cash_flow.entries = t->amount;
        /* Already confirmed by the nodes*/
        self->budget += t->amount;
        return 0;
    } else if (self->pid == t->sender && t->t_type == TRANSACTION_SUCCES) {
        /*outcomes is already updated -> check definition -_- (ricordati i return)*/
        self->budget -= t->amount;
        return 0;
    } else if (self->pid == t->sender && t->t_type == TRANSACTION_FAILED) {
        self->cash_flow.outcomes += t->amount;
        return 0;
    }
    return -1;
}

int generate_transaction(struct user_transaction *self, pid_t user_proc_pid, int *nodes_num, int *users_num) {
    struct Transaction t;
    struct timespec timestamp;

    if (clock_gettime(CLOCK_REALTIME, &timestamp) < 0 || check_balance(self) == FALSE) {
        DEBUG_MESSAGE("IMPOSSIBLE TO RETRIEVE CLOCK_TIME");
        return -1;
    }
    t.t_type = TRANSACTION_WAITING;
    t.hops = 0;
    t.sender = user_proc_pid;
    t.reciver = extract_user(users_num);
    t.timestamp = timestamp.tv_nsec;
    t.amount = gen_amount(self);
    t.reward = 0; /* Is not responsible*/
    queue_append(self->in_process, t);
    self->update_cash_flow(self, &t);
    printf("\n ----------------- timestamp: %lf", t.timestamp);
    return 0;
}

pid_t extract_user(int *users_num) {
    int max = users_num[0];
    int e = (rand() % (max)) + 1;
    while (users_num[e] != NULL && users_num[e] <= 0) {
        e = (rand() % (max)) + 1;
    }
    return users_num[e];
}

pid_t extract_node(int nodes_num) {

}


float gen_amount(struct user_transaction *user) {
    /* TODO: Possible bug*/
    return (rand() % ((int) user->budget - 2 + 1)) + 2;
}