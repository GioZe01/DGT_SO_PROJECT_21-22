#ifndef DGT_SO_PROJECT_21_22_NODE_TP_SHM_H
#define DGT_SO_PROJECT_21_22_NODE_TP_SHM_H

#include "transaction_list.h"
#include "glob.h"

/*! \def IS_EMPTY
 * \brief Empty shm cell
 */
#define IS_EMPTY 1
/*! \def IS_FULL
 * \brief Full shm cell
 */
#define FULL 2
/*! \struct node_block
 * To load this struct it's raccomanded to use a semaphore with a semval set to FULL from the writer
 * and reset to IS_EMPTY from the reader, in order to make data consistent.
 */
struct node_block{
    struct Transaction block_t[SO_BLOCK_SIZE]; /**! Array of transactions */
};
#endif/*DGT_SO_PROJECT_21_22_NODE_TP_SHM_H*/
