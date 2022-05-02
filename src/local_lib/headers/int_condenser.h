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
 * @param max The max value of 1s in the number
 * @return A random number with n 1s and the rest 0
 * @warning rand is done on 32 bits
 */
int rand_int_n(int n, int max);

/**
 * @brief Initialize a integer number with 1 in n random positions excluding the one given and 0 elsewhere
 * @param n The n positions to be set to 1
 * @param exclude The position to be excluded
 * @param max The max value of 1s in the number
 * @return A random number with n 1s and the rest 0
 * @warning rand is done on 32 bits
 */
int rand_int_n_exclude(int n, int exclude, int max);

/**
 * @brief Return the position of a randomly chosen 1 in the given number
 * @param n The number
 * @return The position of a randomly chosen 1 in the given number
 * @warning rand is done on 32 bits
 */
int get_rand_one(int n);

/**
 * @brief Get a random 1 position in the given number
 * @param n The number
 * @return The position of a random 1 in the given number
 */
int rand_int_n_pos(int n);

/**
 * @brief Insert a 1 in the given position in the given number
 * @param n The number
 * @param pos The position
 * @return The number with a 1 in the given position
 */
int set_one(int n, int pos);
#endif //DGT_SO_PROJECT_21_22_INT_CONDENSER_H
