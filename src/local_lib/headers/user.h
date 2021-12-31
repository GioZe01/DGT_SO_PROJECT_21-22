
#ifndef DGT_SO_PROJECT_21_22_USER_H
#define DGT_SO_PROJECT_21_22_USER_H
#include "transaction_list.h"
typedef int Bilancio (unsigned int budget/*TODO: aggiungere transaction list e quelli da fare ancora*/);

typedef struct transaction_list transaction_list;
typedef struct{
    transaction_list *transactions_done;
    transaction_list *in_process;

}User;
#endif /*DGT_SO_PROJECT_21_22_USER_H*/
