
#ifndef DGT_SO_PROJECT_21_22_CONF_SHM_H
#define DGT_SO_PROJECT_21_22_CONF_SHM_H

#include "glob.h"

struct shm_conf {
    int users_snapshots[USERS_MAX][2];
    int nodes_snapshots[NODES_MAX][2];
};

/**
 * Create and initialize the shm structure given
 * @param self the shm struct to be initialized
 * @param users_snapshot the users_info to be saved
 * @param nodes_snapshot the nodes_info to be saved
 * @return -1 in case of FAILURE. 0 otherwise
 */
int
shm_conf_create(struct shm_conf *self, int *users_pids, int *users_queues_ids, int *nodes_pids, int *nodes_queues_ids);

/**
 * Print the shared memory size on console
 * @param self
 */
void shm_conf_print(struct shm_conf *self);

#endif /*DGT_SO_PROJECT_21_22_CONF_SHM_H*/