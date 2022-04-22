
#ifndef DGT_SO_PROJECT_21_22_CONF_SHM_H
#define DGT_SO_PROJECT_21_22_CONF_SHM_H

#include "glob.h"
#include "boolean.h"


struct shm_conf {
    int users_snapshots[USERS_MAX][2];
    int nodes_snapshots[NODES_MAX][3];
};

/**
 * Create and initialize the shm structure given
 * @param self the shm struct to be initialized
 * @param users_snapshot the users_info to be saved
 * @param nodes_snapshot the nodes_info to be saved
 * @return -1 in case of UNKNOWN FAILURE. 0 otherwise
 */
int shm_conf_create(struct shm_conf *self, int *users_pids, int *users_queues_ids, int *nodes_pids, int *nodes_queues_ids);

/**
 * Print the shared memory size on console
 * @param self
 */
void shm_conf_print(struct shm_conf *self);

/**
 * Copy the two pointers given into 2d pointer
 * @param snapshot the 2d matrix
 * @param pids the pointer that's gonna to fill column 0
 * @param queues_ids  the pointer that's gonna fill column 1
 */
void shm_copy_snapshots(int snapshot[][2], int *pids, int *queues_ids);

/**
 * @brief Get the queue id from the given shm conf and the pid
 * TODO: Implement a non linear search
 * @param self the shm conf
 * @param pid the pid to search for
 * @return -1 in case of no pid found, the value otherwise
 */
int get_queueid_by_pid(struct shm_conf *self, int pid, Bool in_users);
#endif /*DGT_SO_PROJECT_21_22_CONF_SHM_H*/
