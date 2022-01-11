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

pid_t extract_user(int users_num);

void user_create(struct user_transaction *u, float budget, int pid, Balance balance) {
    u->pid = pid;
    u->budget = budget;
    u->u_balance = balance;
    u->transactions_done = queue_create();
    u->in_process = queue_create();
    u->entries = budget;
    u->outcomes = 0;
    u->expected_out = 0;
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
    return fabs(self->entries) - fabs(self->outcomes) - fabs(self->expected_out);
}

int generate_transaction(struct user_transaction *self, pid_t user_proc_pid, int nodes_num, int users_num) {
    struct Transaction t;
    struct timespec timestamp;

    if (clock_gettime(CLOCK_REALTIME,&timestamp)<0){
        DEBUG_MESSAGE("IMPOSSIBLE TO RETRIVE CLOCK_TIME");
        return -1;
    }
    t.sender = user_proc_pid;
    t.reciver = extract_user(users_num);
    t.timestamp = timestamp.tv_nsec;
    printf("\n ----------------- timestamp: %lf", t.timestamp);
}
pid_t extract_user(int users_num){
}
pid_t extract_node(int nodes_num){

}