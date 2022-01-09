
#ifndef DGT_SO_PROJECT_21_22_USER_TRANSACTION_H
#define DGT_SO_PROJECT_21_22_USER_TRANSACTION_H

typedef float(*Balance)(/*TODO: aggiungere transaction list e quelli da fare ancora*/);

#include "boolean.h"
#include "transaction_list.h"

struct user_transaction {
    int pid;
    float budget;
    Balance u_balance;
    Queue transactions_done; /*Actually confermed by the nodes_proc -> updated into the shm*/
    Queue in_process;/*Validated, but still to be confirmed by the nodes_proc*/
    float entries;
    float outcomes;
    float expected_out;/*Expected outcome that still have to be processed, abs value*/
};

struct user_snapshot {
    int budget;
};/*Structure to be saved in sharedmemory*/
/*struct user_snapshot* get_user_snapshot(struct user_transaction u);/*TODO: verifica snapshot se ritornare puntatore o no per salvare in shm*/
/**
 * Initiale the user_transaction to default value with entrie set as budget given
 * @return 0 if success
 */
void user_create(struct user_transaction *u, float budget, int pid, Balance balance);

/**
 * Free the memory from the queue list of transactions;
 * @param self
 */
void free_user(struct user_transaction *self);

/**
 *  Check if balance is >= 2
 * @param self
 * @return TRUE if balance >=2, FALSE otherwise
 */
Bool check_balance(struct user_transaction *self);

/**
 * Calculate the balance
 *  * balance = entries - outcomes - expect_out;
 * @param self
 * @return the current balance
 */
float calc_balance(struct user_transaction *self);

/**
 *  Generate a new transaction for the self user and add it to in_progress queue of the transaction
 * @param self
 * @return -1 in case o failure. 0 otherwise.
 */
int generate_transaction(struct user_transaction *self, pid_t user_proc_pid);


/*
struct user_snapshot *get_user_snapshot(struct user_transaction user) {
    TODO: implement get_user_snapshot
}*/

#endif /*DGT_SO_PROJECT_21_22_USER_TRANSACTION_H*/
