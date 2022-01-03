#include <stdio.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include "headers/semaphore.h"


int semaphore_lock(int semaphore_id, short int semaphore_index){
    struct sembuf op_sem;
    op_sem.sem_op = -1;
    op_sem.sem_num = semaphore_index;
    op_sem.sem_flg = 0;
    if(semop(semaphore_id, &op_sem,1)<0)return -1;
    return 0;
}

int semaphore_unlock(int semaphore_id, short int sempahore_index){
    struct sembuf op_sem;
    op_sem.sem_op = 1;
    op_sem.sem_num = sempahore_index;
    op_sem.sem_flg = 0;
    if (semop(semaphore_id, &op_sem, 1)<0) return -1;
    return 0;
}

int semaphore_wait_for_sinc(int semaphore_id, short int semphore_index){
    struct sembuf op_sem;
    op_sem.sem_op = 0;
    op_sem.sem_num = semphore_index;
    op_sem.sem_flg = 0;
    if(semop(semaphore_id, &op_sem, 1)<0)return -1;
    return 0;
}
