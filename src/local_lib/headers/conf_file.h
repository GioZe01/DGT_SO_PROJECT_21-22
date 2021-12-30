

#ifndef DGT_SO_PROJECT_21_22_CONF_FILE_H
#define DGT_SO_PROJECT_21_22_CONF_FILE_H

#define CONFIGURATION_FILE_PATH "./configuration"

struct conf{
    /*TODO: Verificare tipi*/
    /*TODO: Finire di scrivere la configurazione -> potrebbe arrivare da socket*/
    unsigned int so_user_num;
    unsigned int so_nodes_num;
    unsigned int so_buget_init;
    float so_reward;
    unsigned long int so_min_trans_gen_nsec;
    unsigned long int so_max_trans_gen_nsec;
    unsigned int so_retry;
    unsigned int so_tp_size;
    unsigned long int so_min_trans_proc_nsec;
    unsigned long int so_max_trans_proc_nsec;
    unsigned long int so_sim_sec;
    /*TODO: aggiungere per versione max 30*/
};
 /**
  * \brief Load the configuration in the conf struct specified
  * Load the configuration present in the configuration file
  * \attentionWarning Negative number are automatically converted to positive !
  * @param self
  * @return
  * Return Types:
  * 1) -1 if file absent or empty for some filds required for conf
  * 2) -2 if a params does't respect type def. or some internal logic of the simulations ex: no budgets
  * 3) -3 if at least two users and one node
  * 4) -4 min max not respected in time params
  * 5) -5 if node rewards is max for budget init; TODO: vedere se si vuole bloccare fin da subito considerando worst case e se negative buget safe è da fare o no
  */
int load_configuration(struct conf* self);
#endif /*DGT_SO_PROJECT_21_22_CONF_FILE_H*/
