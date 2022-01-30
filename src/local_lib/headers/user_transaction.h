
#ifndef DGT_SO_PROJECT_21_22_USER_TRANSACTION_H
#define DGT_SO_PROJECT_21_22_USER_TRANSACTION_H

#include "boolean.h"
#include "transaction_list.h"
#include "conf_shm.h"
typedef float(*Balance)(struct user_transaction* self);

typedef int (*CalcCashFlow)(struct user_transaction*self, struct Transaction *t);

typedef struct UCashFlow {
    float entries;
    float outcomes;
};
struct user_transaction {
    int pid;
    float budget; /*Contains the confirmed by nodes value in the pocket of the current user*/
    Balance u_balance;
    Queue transactions_done; /*Actually confermed by the nodes_proc -> updated into the shm*/
    Queue in_process;/*Validated, but still to be confirmed by the nodes_proc*/
    struct UCashFlow cash_flow;
    CalcCashFlow update_cash_flow; /*Can set the function u desire while implementing, consider the one already implemented below*/
    float expected_out; /* Expected outcomes to still be processed*/
    int to_wait_transaction;/*Num Transactions to waite on for the conformation of processing */
};

struct user_snapshot {
    int pid;
    int budget;
    float u_balance;
    float entries;
    float outcomes;
    float expected_out;
};/*Structure to be saved in sharedmemory*/
/*struct user_snapshot* get_user_snapshot(struct user_transaction self);/*TODO: verifica snapshot se ritornare puntatore o no per salvare in shm*/
/**
 * Initialize the user_transaction to default value with entries set as budget given
 * @param self current user
 * @param budget to start
 * @param pid current proc_pid
 * @param balance pointer to function that calc balance
 * @param update_cash_flow pointer to function that update cash flow | and budget!!
 */
void user_create(struct user_transaction *self, float budget, int pid, Balance balance, CalcCashFlow update_cash_flow);

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
 * Update entries, outcomes and budget based on transaction
 * @param t the transaction arrived
 * @param self current user
 * @return -1 in case of failure, 0 otherwise
 */
int update_cash_flow(struct user_transaction *self, struct Transaction *t);

/**
 * Generate a new transaction for the self user and add it to in_progress queue of the transaction
 * @param self pointer to the transaction to be generated
 * @param user_proc_pid  pid of the user generating the transaction
 * @param users_num pointer to a vector of users available
 * @return -1 in case o failure. 0 otherwise.
 */
int generate_transaction(struct user_transaction *self, pid_t user_proc_pid,struct shm_conf *shm_conf);


/*
struct user_snapshot *get_user_snapshot(struct user_transaction user) {
    TODO: implement get_user_snapshot
}*/

#endif /*DGT_SO_PROJECT_21_22_USER_TRANSACTION_H*/
