
#ifndef DGT_SO_PROJECT_21_22_USER_TRANSACTION_H
#define DGT_SO_PROJECT_21_22_USER_TRANSACTION_H
typedef int (*Balance)(unsigned int budget/*TODO: aggiungere transaction list e quelli da fare ancora*/);

#include "transaction_list.h"
struct user_transaction{
    int pid;
    int budget;
    Balance u_balance;
    Queue transactions_done; /*Actually confermed by the nodes_proc -> updated into the shm*/
    Queue in_process;/*Validated, but still to be confirmed by the nodes_proc*/
} ;

struct user_snapshot{
    int budget;
};/*Structure to be saved in sharedmemory*/
/*struct user_snapshot* get_user_snapshot(struct user_transaction u);/*TODO: verifica snapshot se ritornare puntatore o no per salvare in shm*/
/**
 * Initiale the user_transaction to default value
 * @return 0 if success
 */
user_transaction user_create(int budget,int pid, Balance balance);
/**
 * Free the memory from the queue list of transactions;
 * @param self
 */
void free_user(struct user_transaction self);
#endif /*DGT_SO_PROJECT_21_22_USER_TRANSACTION_H*/
