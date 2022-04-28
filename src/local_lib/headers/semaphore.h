
#ifndef DGT_SO_PROJECT_21_22_SEMAPHORE_H
#define DGT_SO_PROJECT_21_22_SEMAPHORE_H

/**
 * Execute a lock on semaphore_index of semaphore_id
 * array
 * @param semaphore_id array of semaphores indexs
 * @param semaphore_index index of the semaphore to exe the lock on
 * @return 0 in case of success, -1 otherwise
 */
int semaphore_lock(int semaphore_id, int semaphore_index);

/**
 * Execute an unlock on sempaphore_index of semaphore_id array
 * @param semaphore_id array of semaphores indexs
 * @param semaphore_index index of the semaphore to exe the lock on
 * @return 0 in case of success, -1 otherwise
 */
int semaphore_unlock(int semaphore_id, int sempahore_index);

/**
 * Execute a wait for zero on sempaphore_index of semaphore_id array
 * @param semaphore_id array of semaphores indexs
 * @param semaphore_index index of the semaphore to exe the lock on
 * @return 0 in case of success, -1 otherwise
 */
int semaphore_wait_for_sinc(int semaphore_id, int semaphore_index);

#endif /*DGT_SO_PROJECT_21_22_SEMAPHORE_H*/
