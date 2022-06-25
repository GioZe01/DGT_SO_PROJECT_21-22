/**
 * \file conf_file.h
 * \brief Header file for the configuration file parser conf_file.c
 * \author Giovanni Terzuolo
 */

#ifndef DGT_SO_PROJECT_21_22_CONF_FILE_H
#define DGT_SO_PROJECT_21_22_CONF_FILE_H

#include "boolean.h"

#define CONFIGURATION_FILE_PATH "/home/golia/CLionProjects/DGT_SO_PROJECT_21-22/conf"

struct conf {
    int so_user_num; /**< The number of users */
    int so_nodes_num; /**< The number of nodes */
    float so_buget_init; /**< The initial budget */
    float so_reward; /**< The reward for each node */
    long so_min_trans_gen_nsec; /**< Min time for transaction generation*/
    long so_max_trans_gen_nsec; /**< Max time for transaction generation*/
    int so_retry; /**< Number of retry before failing after sending a transaction to a node*/
    int so_tp_size; /**< Transaction pool size*/
    long so_min_trans_proc_nsec; /**< Min time for transaction processing*/
    long so_max_trans_proc_nsec; /**< Max time for transaction processing*/
    long so_sim_sec; /**< Execution time of the simulation*/
    short int so_num_friends; /**< Number of node's friends*/
    int so_hops; /**< Number of hops*/
};

/**
 * @brief Load the configuration in the conf struct specified
 * Load the configuration present in the configuration file
 * \attentionWarning Negative number are automatically converted to positive !
 * @param self
 * @return
 * Return Types:
 * 1) -1 if file absent or empty for some filds required for conf
 * 2) -2 if a params does't respect type def. or some internal logic of the simulations ex: no budgets
 * 3) -3 if at least two users and one node and zero hops
 * 4) -4 min max not respected
 * 5) -5 if node rewards is max for budget init. Or so_num_friends > MAX_FRIENDS
 */
int load_configuration(struct conf *self);

#endif /*DGT_SO_PROJECT_21_22_CONF_FILE_H*/
