#ifndef DGT_SO_PROJECT_21_22_BOOK_MASTER_SHM_H
#define DGT_SO_PROJECT_21_22_BOOK_MASTER_SHM_H
/*Local Import*/
#include "transaction_list.h"
#include "glob.h"
union Block{
    struct Transaction block_t [SO_BLOCK_SIZE];
    int index;
};
struct shm_book_master{
    union Block blocks [SO_REGISTRY_SIZE][2];
    int to_fill; /*index free to be filled (need his own dedicated semaphore)*/
};
/*          0                   1
 * 0    block_transactions      index   <-  ;
 * */
/**
 * Load all the index in col 2 of blocks
 * @param self the shm to create
 * @param sem_access_id the id of the semaphore to access the shm value
 * @return -1 in case of Failure. 0 otherwise
 */
int shm_book_master_create(struct shm_book_master * self, int sem_access_id);
/**
 * Print at console the given book_master
 * @param self the book_master to be printed
 */
void shm_book_master_print(struct shm_book_master * self);
/**
 * Load into the given shm struct the block.
 * @param self the shm struct to load the block in
 * @param block_t block of transactions to be loaded
 * @return -1 in case of FAILURE. 0 otherwise
 */
int insert_block(struct shm_book_master * self, struct Transaction block_t[SO_BLOCK_SIZE]);
#endif /*DGT_SO_PROJECT_21_22_BOOK_MASTER_SHM_H*/
