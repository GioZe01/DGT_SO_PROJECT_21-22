

#ifndef DGT_SO_PROJECT_21_22_CONF_FILE_H
#define DGT_SO_PROJECT_21_22_CONF_FILE_H

#include "boolean.h"

#define CONFIGURATION_FILE_PATH "conf"

struct conf
{
    /*TODO: Verificare tipi*/
    /*TODO: Finire di scrivere la configurazione -> potrebbe arrivare da socket*/
    int so_user_num;
    int so_nodes_num;
    float so_buget_init;
    float so_reward;
    long so_min_trans_gen_nsec;
    long so_max_trans_gen_nsec;
    int so_retry;
    int so_tp_size;
    long so_min_trans_proc_nsec;
    long so_max_trans_proc_nsec;
    long so_sim_sec;
    short int so_num_friends;
    int so_hops;
    /*TODO: aggiungere per versione max 30*/
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
 * 3) -3 if at least two users and one node
 * 4) -4 min max not respected in time params
 * 5) -5 if node rewards is max for budget init. Or so_num_friends > MAX_FRIENDS
 */
int load_configuration(struct conf *self);

#endif /*DGT_SO_PROJECT_21_22_CONF_FILE_H*/
