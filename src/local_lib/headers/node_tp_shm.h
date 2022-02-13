#ifndef DGT_SO_PROJECT_21_22_NODE_TP_SHM_H
#define DGT_SO_PROJECT_21_22_NODE_TP_SHM_H
#include "transaction_list.h"
#include "glob.h"
/**
 * To load this struct it's raccomanded to use a semaphore with a semval set to 1 from the writer
 * and reset to 0 from the reader, in order to make data consistent.
 */
struct node_block{
    struct Transaction block_t[SO_BLOCK_SIZE];
};
#endif/*DGT_SO_PROJECT_21_22_NODE_TP_SHM_H*/
