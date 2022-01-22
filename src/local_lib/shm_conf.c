/* Std*/
#include <stdio.h>
#include <stdlib.h>
/* Local*/
#include "headers/conf_shm.h"
#include "headers/boolean.h"

/* Helper function*/


shm_conf_create(struct shm_conf *self, int *users_pids, int *users_queues_ids, int *nodes_pids, int *nodes_queues_ids) {
    shm_copy_snapshots(self->users_snapshots, users_pids, users_queues_ids, TRUE);
    shm_copy_snapshots(self->nodes_snapshots, nodes_pids, nodes_queues_ids, TRUE);
    return 0;
}

void shm_conf_print(struct shm_conf *self) {
    printf("===============CONFIGURATION===============\n");
    printf("|| users_snapshots: %d\n", self->users_snapshots[0][0]);
    printf("|| nodes_snapshots: %d\n", self->nodes_snapshots[0][0]);
    printf("===========================================\n");
}

void shm_copy_snapshots(int snapshot[][2], int *pids, int *queues_ids, Bool to_free) {
    int row, column;
    for (row = 0; row < pids[0]+1; row++) {
        for (column = 0; column < 2; column++) {
            if (column == 0)snapshot[row][column] = pids[row];
            if (column == 1) snapshot[row][column] = queues_ids[row];
        }
    }
    if (to_free == TRUE) {
        free(pids);
        free(queues_ids);
    }
}