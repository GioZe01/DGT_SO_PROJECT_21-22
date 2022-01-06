#include <stdio.h>
#include <stdlib.h>
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

void user_create(struct user_transaction *u, int budget, int pid, Balance balance) {
    u->pid = pid;
    u->budget = budget;
    u->u_balance = balance;
    u->transactions_done = queue_create();
    u->in_process = queue_create();
}

void free_user(struct user_transaction self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE USER OPERATION...");
    queue_destroy(self.in_process);
    queue_destroy(self.transactions_done);
    DEBUG_NOTIFY_ACTIVITY_RUNNING("FREE USER OPERATION DONE");
}