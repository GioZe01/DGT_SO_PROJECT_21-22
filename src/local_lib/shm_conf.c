/* Std*/
#include <stdio.h>
/* Local*/
#include "headers/conf_shm.h"
#include "headers/boolean.h"

/* Helper function*/


int shm_conf_create(struct shm_conf *self, int *users_pids, int *users_queues_ids, int *nodes_pids, int *nodes_queues_ids, int * nodes_friends) {
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
void shm_copy_snapshots_users(int snapshot[][2], int *pids, int *queues_ids){
    int row, column;
    for (row = 0; row < pids[0]+1; row++) {
        for (column = 0; column < 2; column++) {
            if (column == 0)snapshot[row][column] = pids[row];
            if (column == 1) snapshot[row][column] = queues_ids[row];
        }
    }
}

void shm_copy_snapshots_nodes(int snapshot[][3], int *pids, int *queues_ids, int *friends){
    int row, column;
    for (row = 0; row < pids[0]+1; row++) {
        for (column = 0; column < 2; column++) {
            if (column == 0)snapshot[row][column] = pids[row];
            if (column == 1) snapshot[row][column] = queues_ids[row];
            if (column == 2) snapshot[row][column] = friends[row];
        }
    }
}

int get_queueid_by_pid(struct shm_conf *self, int pid, Bool in_users){
    int *snapshot;
    int ris = -1;
    if (in_users == TRUE){
        snapshot = &self->users_snapshots[0][0];
        for(;snapshot != NULL; snapshot+=2){
            if (*snapshot == pid){
                ris = *(snapshot+1);
                return ris;
            }
        }
    }else{
        snapshot = &self->nodes_snapshots[0][0];
        for(;snapshot != NULL; snapshot+=3){
            if (*snapshot == pid){
                ris = *(snapshot+1);
                return ris;
            }
        }

    }
    return ris;
}

int get_node_position_by_pid(struct shm_conf *self, int pid){
    int *snapshot;
    int ris = -1;
    snapshot = &self->nodes_snapshots[0][0];
    for(;snapshot != NULL; snapshot+=3){
        if (*snapshot == pid){
            ris = *(snapshot+1);
            return ris;
        }
    }
    return ris;
}
