/* Std*/
#include <stdio.h>
#include <stdlib.h>
/* Local*/
#include "headers/conf_shm.h"
#include "headers/boolean.h"

/* Helper function*/


int
shm_conf_create(struct shm_conf *self, int *users_pids, int *users_queues_ids, int *nodes_pids, int *nodes_queues_ids,
                int *nodes_friends) {
    shm_copy_snapshots_users(self->users_snapshots, users_pids, users_queues_ids);
    shm_copy_snapshots_nodes(self->nodes_snapshots, nodes_pids, nodes_queues_ids, nodes_friends);
    return 0;
}

void shm_conf_print(struct shm_conf *self) {
    printf("===============CONFIGURATION===============\n");
    printf("|| users_snapshots: %d\n", self->users_snapshots[0][0]);
    printf("|| nodes_snapshots: %d\n", self->nodes_snapshots[0][0]);
    printf("===========================================\n");
}

void shm_copy_snapshots_users(int (*snapshot)[2], int *pids, int *queues_ids) {
    int row;
    for (row = 0; row < pids[0] + 1; row++) {
        snapshot[row][0] = pids[row];
        snapshot[row][1] = queues_ids[row];
    }
}

void shm_copy_snapshots_nodes(int (*snapshot)[3], int *pids, int *queues_ids, int *friends) {
    int row;
    for (row = 0; row < pids[0] + 1; row++) {
        snapshot[row][0] = pids[row];
        snapshot[row][1] = queues_ids[row];
        snapshot[row][2] = friends[row];
    }
}

int get_queueid_by_pid(struct shm_conf *self, int pid, Bool in_users) {
    int *snapshot;
    int ris = -1;
    if (in_users == TRUE) {
        snapshot = &self->users_snapshots[1][0];
        for (; snapshot != NULL; snapshot += 2) {
            if (*snapshot == pid) {
                ris = *(snapshot + 1);
                return ris;
            }
        }
    } else {
        snapshot = &self->nodes_snapshots[1][0];
        for (; snapshot != NULL; snapshot += 3) {
            if (*snapshot == pid) {
                ris = *(snapshot + 1);
                return ris;
            }
        }

    }
    return ris;
}

int get_node_position_by_pid(struct shm_conf *self, int pid) {
    int *snapshot;
    int ris = -1;
    snapshot = &(self->nodes_snapshots[0][0]);
    for (ris = 0; snapshot != NULL; snapshot += 3) {
        if (*snapshot == pid) {
            return ris;
        }
        ris++;
    }
    return ris;
}

Bool shm_conf_add_node(struct shm_conf *self, int pid, int queue_id, int friends) {
    if (self->nodes_snapshots[0][0] == NODES_MAX) {
        return FALSE;
    }
    /**
     * Check if queue_id is already used
     */
    int *snapshot;
    snapshot = &self->nodes_snapshots[1][0];
    int i = 0;
    for (i = 0; i < self->nodes_snapshots[0][0]; snapshot += 3) {
        if (*(snapshot + 1) == queue_id) {
            return FALSE;
        }
        i++;
    }
    /**
     * Add the node information to the snapshot at snapshot[0][0] +1 position
     */
    int row = self->nodes_snapshots[0][0] + 1;
    self->nodes_snapshots[row][0] = pid;
    self->nodes_snapshots[row][1] = queue_id;
    self->nodes_snapshots[row][2] = friends;
    self->nodes_snapshots[0][0]++;
    return TRUE;
}
