/* Std*/

/* Local*/
#include "headers/conf_shm.h"
/* Helper function*/
/**
 * Copy the content of a pointer into another
 * @param where to be stored
 * @param to_copy array to be copied
 */
void copy_snapshots(int *where, int *to_copy, int max);


int shm_conf_create(struct shm_conf *self, int users_snapshots[][2], int nodes_snapshots[][2]) {
    copy_snapshots(&self->nodes_snapshots[0][0], &nodes_snapshots[0][0],
                   (nodes_snapshots[0][0] + 1) < NODES_MAX ? nodes_snapshots[0][0] : NODES_MAX);
    copy_snapshots(&self->users_snapshots[0][0], &users_snapshots[0][0],
                   (nodes_snapshots[0][0] + 1) < NODES_MAX ? nodes_snapshots[0][0] : USERS_MAX);
    return 0;
}

void copy_snapshots(int *where, int *to_copy, int max) {
    int i;
    for (i = 0; i < max; i++) {
        *(where + i) = *(to_copy + i);
    }
}