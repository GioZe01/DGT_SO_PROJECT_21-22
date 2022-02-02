#ifndef DGT_SO_PROJECT_21_22_BOOK_MASTER_SHM_H
#define DGT_SO_PROJECT_21_22_BOOK_MASTER_SHM_H
/*Local Import*/
#include "transaction_list.h"
#include "glob.h"
union Block{
    Queue block_transactions;
    int index;
};
struct shm_book_master{
    union Block blocks [SO_REGISTRY_SIZE][2];
};

/*          0                   1
 * 0    block_transactions      index
 * */
#endif /*DGT_SO_PROJECT_21_22_BOOK_MASTER_SHM_H*/
