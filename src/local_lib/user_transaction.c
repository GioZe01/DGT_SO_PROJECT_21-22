#include <stdio.h>
#include <stdlib.h>
#include "headers/user_transaction.h"


struct user_transaction user_create(int budget, int pid, Balance balance) {
    struct user_transaction u;
    u.pid = pid;
    u.budget = budget;
    u.u_balance = balance;
    u.transactions_done = queue_create();
    u.in_process = queue_create();
    return u;
}