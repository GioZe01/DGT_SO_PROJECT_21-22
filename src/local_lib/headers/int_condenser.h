#ifndef DGT_SO_PROJECT_21_22_INT_CONDENSER_H
#define DGT_SO_PROJECT_21_22_INT_CONDENSER_H
/**
 * @brief Generate a random number between 0 and n-1
 * @param n The upper bound
 * @return A random number between 0 and n-1
 */
int rand_int(int n);

/**
 * @brief Generate a random number between min and max
 * @param min The lower bound
 * @param max The upper bound
 * @return A random number between min and max
 */
int rand_int_range(int min, int max);

/**
 * @brief Get the 1s positions of a number
 * @param n The number
 * @return The positions of 1s
 */
int get_ones(int n);

/**
 * @brief Initialize a integer number with 1 in n random positions and 0 elsewhere
 * @param n The n positions to be set to 1
 * @return A random number with n 1s and the rest 0
 */
int rand_int_n(int n);
#endif //DGT_SO_PROJECT_21_22_INT_CONDENSER_H
