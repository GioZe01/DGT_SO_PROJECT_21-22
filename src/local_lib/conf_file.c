
#include <stdio.h>
#include "headers/conf_file.h"


/*TODO:scrivere funzione di caricamento configurazione e vedere se manca qualcosa*/
int load_configuration(struct conf *self) {
    FILE *conf_file;

    conf_file = fopen(CONFIGURATION_FILE_PATH, "r");
    /*Checks for -1*/
    if (conf_file == NULL) return -1;
    if (fscanf(conf_file, "so_user_num=%u", &self->so_user_num) == EOF) return -1;
    if (fscanf(conf_file, "so_nodes_num=%u", &self->so_nodes_num) == EOF) return -1;
    if (fscanf(conf_file, "so_budget_init=%u", &self->so_buget_init) == EOF) return -1;
    if (fscanf(conf_file, "so_reward=%f", &self->so_reward)== EOF) return -1;
    if (fscanf(conf_file, "so_min_trans_gen_nsec=%lu", &self->so_min_trans_gen_nsec) == EOF) return -1;
    if (fscanf(conf_file, "so_max_trans_gen_nsec=%lu", &self->so_max_trans_gen_nsec) == EOF) return -1;
    if (fscanf(conf_file, "so_retry=%u", &self->so_retry) == EOF) return -1;
    if (fscanf(conf_file, "so_tp_size=%u", &self->so_tp_size) == EOF) return -1;
    if (fscanf(conf_file, "so_min_trans_proc_nsec=%lu", &self->so_min_trans_proc_nsec) == EOF) return -1;
    if (fscanf(conf_file, "so_max_trans_proc_nsec=%lu", &self->so_max_trans_proc_nsec) == EOF) return -1;
    if (fscanf(conf_file, "so_sim_sec=%lu", &self->so_sim_sec) == EOF) return -1;
    /*Checks for -2*/
    if (self->so_user_num == 0 || self->so_nodes_num == 0 || self->so_buget_init == 0 ||
        self->so_reward < 0 || self->so_min_trans_gen_nsec == 0 || self->so_max_trans_gen_nsec == 0 ||
        self->so_tp_size == 0 || self->so_sim_sec == 0 ||
        self->so_min_trans_proc_nsec == 0 || self->so_max_trans_proc_nsec == 0)
        return -2;
    /*Checks for -3*/
    if (self->so_user_num < 2 || self->so_nodes_num < 1)/*TODO: numbers nodes check requested ?*/
        return -3;
    /*Checks for -4*/
    if (self->so_min_trans_proc_nsec > self->so_max_trans_proc_nsec ||
        self->so_min_trans_gen_nsec > self->so_max_trans_gen_nsec)
        return -4;
    if (self->so_reward > 1) /*Vorrebbe dire che con una transazione da max budget non puoi pagare le spese*/
        return -5;
    return 0;
}
