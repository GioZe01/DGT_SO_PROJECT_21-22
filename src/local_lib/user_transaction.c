/*  Standard    */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
/*  Local   */
#include "headers/user_transaction.h"
#include "headers/simulation_errors.h"
#include "headers/conf_shm.h"

#ifdef DEBUG

#include "headers/debug_utility.h"

#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

/**
 * Extract randomly a node id from 0 up to nodes_num
 * @param nodes_num limit for random extraction
 * @return the random id extracted
 */
int extract_node(int nodes_num);

/**
 * Select a user pid from the vector pointer users_num randomly
 * @param users_num the pointer to the vector of users_pids
 * @return the pid_t of the user selected randomly
 */
pid_t extract_user(int users_num[][2]);

/**
 * \brief Generate a random float number,
 * representing the amount of a transaction with srand() level with getpid
 * @param user the adt of the user to extract the random amount from
 * @return the amount generated randomly
 */
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
    self->to_wait_transaction = 0;
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
    if (self->pid == t->reciver && t->t_type == TRANSACTION_SUCCES) { /*Getting reaches*/
        self->cash_flow.entries += t->amount;
        /* Already confirmed by the nodes*/
        self->budget += t->amount;
        return 0;
    } else if (self->pid == t->sender && t->t_type == TRANSACTION_SUCCES) {
        /*outcomes is already updated -> check definition*/
        self->budget -= t->amount;
        self->cash_flow.outcomes += t->amount;
        self->expected_out -= t->amount;
        return 0;
    } else if (self->pid == t->sender && t->t_type == TRANSACTION_FAILED) {
        self->cash_flow.outcomes -= t->amount;
        return 0;
    } else if (self->pid == t->sender && t->t_type == TRANSACTION_WAITING) {
        self->expected_out += t->amount;
        return 0;
    }
    return -1;
}

int generate_transaction(struct user_transaction *self, pid_t user_proc_pid, struct shm_conf *shm_conf) {
    /* DEBUG_NOTIFY_ACTIVITY_RUNNING("GENERATING THE TRANSACTION..."); */
    struct Transaction t;
    if (check_balance(self) == TRUE) {
        if (create_transaction(&t, user_proc_pid, extract_user(shm_conf->users_snapshots), gen_amount(self)) <
            0) { ERROR_MESSAGE("FAILED ON TRANSACTION CREATION"); }
/*
#ifdef DEBUG
        transaction_print(t);
#endif
 */
        queue_append(self->in_process, t);
        if(self->update_cash_flow(self, &t)<0){ ERROR_EXIT_SEQUENCE_USER("IMPOSSIBLE TO UPDATE CASH FLOW");}
        self->to_wait_transaction++;
        /* DEBUG_NOTIFY_ACTIVITY_DONE("GENERATING THE TRANSACTION DONE"); */
        return 0;
    }
    return -1;
}

pid_t extract_user(int users_num[][2]) {
    srand(getpid());
   /* DEBUG_NOTIFY_ACTIVITY_RUNNING("EXTRACTING USER FROM SNAPSHOTS...");*/
    int max = users_num[0][0];
    int e = (rand() % (max)) + 1;
    while (users_num[e + 1][0] == NULL) {
        e = (rand() % (max)) + 1;
    }
    /*DEBUG_NOTIFY_ACTIVITY_DONE("EXTRACTING USER FROM SNAPSHOTS DONE");*/
    return users_num[e][0];
}

int extract_node(int nodes_num) {

}


float gen_amount(struct user_transaction *user) {
    srand(getpid());
    /* TODO: Possible bug*/
    return (float) (rand() % (((int) user->budget) - 2 + 1)) + 2;
}
