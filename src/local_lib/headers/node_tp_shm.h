#ifndef DGT_SO_PROJECT_21_22_NODE_TP_SHM_H
#define DGT_SO_PROJECT_21_22_NODE_TP_SHM_H
#include "transaction_list.h"
#include "glob.h"
/* Can be implemented in one variable using bitwise shifting, for reading purpose it's implemented
 * in the following way
 * */
#define IS_EMPTY 0
#define FULL 1
/**
 * To load this struct it's raccomanded to use a semaphore with a semval set to FULL from the writer
 * and reset to IS_EMPTY from the reader, in order to make data consistent.
 */
struct node_block{
    struct Transaction block_t[SO_BLOCK_SIZE];
};
#endif/*DGT_SO_PROJECT_21_22_NODE_TP_SHM_H*/
