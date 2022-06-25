/**
 * \file conf_shm.h
 * \brief Header file for shm_conf.c
 * Define the shared memory structure and the functions to create and destroy the shm that allow the configuration to be shared between the processes
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_CONF_SHM_H
#define DGT_SO_PROJECT_21_22_CONF_SHM_H

#include "glob.h"
#include "boolean.h"

/**\struct conf_shm
 * @brief The configuration of the shared memory
 * Is the configuration used by each node and user process to akcnowledge and process transactions
 * and inter process communication
 *  - users_snapshot: column [0] is the user pid, column [1] is the queue_id
 *  - nodes_snapshot: column [0] is the node pid, column [1] is the queue_id, column [2] is the node_friends_num
 *  \warning node_friends_num is the number of friends of the node, so need to be embedded into an int value \ref #int_condenser.h
 */
struct shm_conf {
    int users_snapshots[USERS_MAX][2]; /**< The snapshots of the users */
    int nodes_snapshots[NODES_MAX][3]; /**< The snapshots of the nodes */
};

/**
 * Create and initialize the shm structure given
 * each parameter pointer is a pointer to a vector of int
 * @param self the shm struct to be initialized
 * @param users_pids the pids of the users
 * @param nodes_pids the pids of the nodes
 * @param users_queues_ids the ids of the users queues
 * @param nodes_queues_ids the ids of the nodes queues
 * @param nodes_friends the ids of the nodes friends
 * @return -1 in case of UNKNOWN FAILURE. 0 otherwise
 */
int
shm_conf_create(struct shm_conf *self, int *users_pids, int *users_queues_ids, int *nodes_pids, int *nodes_queues_ids,
                int *nodes_friends);

/**
 * Print the shared memory size on console
 * @param self the shm struct to be printed
 */
void shm_conf_print(struct shm_conf *self);

/**
 * Copy the two pointers given into 2d pointer
 * @param snapshot pointer to the users snapshot
 * @param pids the pointer that's gonna to fill column 0
 * @param queues_ids  the pointer that's gonna fill column 1
 */
void shm_copy_snapshots_users(int (*snapshot)[2], int *pids, int *queues_ids);

/**
 * Copy the two pointers given into 3d pointer
 * @param snapshot pointer to the nodes snapshot
 * @param pids the pointer that's gonna to fill column 0
 * @param queues_ids  the pointer that's gonna fill column 1
 * @param friends the pointer that's gonna fill column 2. If NULL is not filled the column
 */
void shm_copy_snapshots_nodes(int (*snapshot)[3], int *pids, int *queues_ids, int *friends);

/**
 * @brief Get the queue id from the given shm conf and the pid
 * TODO: Implement a non linear search
 * @param self the shm conf
 * @param pid the pid to search for
 * @return -1 in case of no pid found, the value otherwise
 */
int get_queueid_by_pid(struct shm_conf *self, int pid, Bool in_users);

/**
 * @brief get the index of the node in the shm conf
 * @param self the shm conf
 * @param pid the pid to search for
 * @return -1 in case of no pid found, the value otherwise
 */
int get_node_position_by_pid(struct shm_conf *self, int pid);

/**
 * @brief Add a new node to the shm conf nodes_snapshots
 * @param self the shm conf
 * @param pid the pid of the node
 * @param queue_id the queue id of the node
 * @param friends the friends of the node
 * @return TRUE in case of success, FALSE otherwise
 */
Bool shm_conf_add_node(struct shm_conf *self, int pid, int queue_id, int friends);

#endif /*DGT_SO_PROJECT_21_22_CONF_SHM_H*/
