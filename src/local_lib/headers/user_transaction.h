/**
 * \file user_transaction.h
 * \brief Header file for user_transaction.c
 * Contains the definition of the struct user_transaction and the functions used to manipulate it.
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_USER_TRANSACTION_H
#define DGT_SO_PROJECT_21_22_USER_TRANSACTION_H

/* Local import */
#include "boolean.h"
#include "transaction_list.h"
#include "conf_shm.h"

/**
 * \typedef Balance function pointer
 * \brief Function pointer for the balance function of a user.
 * \param [in] self user
 */
typedef double (*Balance)(struct user_transaction *self);

/**
 * \typedef CalcCashFlow function pointer
 * \brief Function pointer for the calcCashFlow function of a user, based on the transaction given as parameter.
 * \param [in] self user
 * \param [in] t transaction to be used to calculate the cash flow
 */
typedef int (*CalcCashFlow)(struct user_transaction *self, struct Transaction t);

/**
 * \typedef UCashFlow struct
 * \brief Structure used to represent the cash flow of a user.
 */
typedef struct {
    float entries; /**< Cash flow of the user in entries */
    float outcomes; /**< Cash flow of the user in outcomes */
} UCashFlow;
/**
 * \struct user_transaction
 * \brief Structure used to represent a user.
 */
struct user_transaction {
    int pid; /**< PID of the user proc */
    short int exec_state; /**<Current execution state of the user proc*/
    float budget; /**<Contains the confirmed by nodes value in the pocket of the current user*/
    Balance u_balance; /**<Function pointer to the balance function of the user*/
    Queue transactions_failed; /**< Transactions_failed not processed by any node*/
    Queue in_process;/**< Validated, but still to be confirmed by the nodes_proc*/
    UCashFlow cash_flow; /**< Cash flow of the user*/
    CalcCashFlow update_cash_flow; /**< Function pointer to the calcCashFlow function of the user*/
    float expected_out; /**< Expected outcomes to still be processed*/
    int to_wait_transaction;/**< Num Transactions to wait for the conformation of processing */
};

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
double calc_balance(struct user_transaction *self);

/**
 * Print the cashflow info
 * @param self the user to be printed
 */
void print_cashflow(struct user_transaction *self);

/**
 * Update entries, outcomes and budget based on transaction
 * @param t the transaction arrived
 * @param self current user
 * @return -1 in case of failure, 0 otherwise
 */
int update_cash_flow(struct user_transaction *self, struct Transaction t);

/**
 * Generate a new transaction for the self user and add it to in_progress queue of the transaction
 * @param self pointer to the transaction to be generated
 * @param user_proc_pid  pid of the user generating the transaction
 * @param users_num pointer to a vector of users available
 * @return -1 in case o failure. 0 otherwise.
 */
int generate_transaction(struct user_transaction *self, pid_t user_proc_pid, struct shm_conf *shm_conf);

/**
 * Remove the transaction from the list
 * @param q the transaction list
 * @param t the transaction to check for remove
 * @return -1 in case of Failure, -2 in case of empty queue. 0 otherwise.
 */
int queue_remove(Queue q, struct Transaction t);

/**
 * Extract randomly a node id from 0 up to nodes_num
 * @param nodes_num limit for random extraction
 * @return the random id extracted
 */
int extract_node(int nodes_num);

#endif /*DGT_SO_PROJECT_21_22_USER_TRANSACTION_H*/
