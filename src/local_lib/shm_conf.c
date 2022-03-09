/* Std*/
#include <stdio.h>
/* Local*/
#include "headers/conf_shm.h"
#include "headers/boolean.h"

/* Helper function*/


int shm_conf_create(struct shm_conf *self, int *users_pids, int *users_queues_ids, int *nodes_pids, int *nodes_queues_ids) {
    shm_copy_snapshots(self->users_snapshots, users_pids, users_queues_ids);
    shm_copy_snapshots(self->nodes_snapshots, nodes_pids, nodes_queues_ids);
    return 0;
}

void shm_conf_print(struct shm_conf *self) {
    printf("===============CONFIGURATION===============\n");
    printf("|| users_snapshots: %d\n", self->users_snapshots[0][0]);
    printf("|| nodes_snapshots: %d\n", self->nodes_snapshots[0][0]);
    printf("===========================================\n");
}

void shm_copy_snapshots(int snapshot[][2], int *pids, int *queues_ids){
    int row, column;
    for (row = 0; row < pids[0]+1; row++) {
        for (column = 0; column < 2; column++) {
            if (column == 0)snapshot[row][column] = pids[row];
            if (column == 1) snapshot[row][column] = queues_ids[row];
        }
    }
}

int get_queueid_by_pid(struct shm_conf *self, int pid, Bool in_users){
    printf("\nCALLLED ......................................................... \n");
    int *snapshot;
    int ris = -1;
    if (in_users == TRUE){
        snapshot = &self->users_snapshots[0][0];
    }else{
        snapshot = &self->nodes_snapshots[0][0];
    }
    printf("\n FOR VALUE : %d \n", *snapshot);
    for(;snapshot != NULL; snapshot+=2){
        printf("\n FOR VALUE : %d \n", *snapshot);
        if (*snapshot == pid){
            ris = *(snapshot+1);
            return ris;
        }
    }
    return ris;
}

