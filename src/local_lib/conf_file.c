#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "headers/conf_file.h"
#include "headers/debug_utility.h"
#include "headers/simulation_errors.h"
#include "headers/boolean.h"

int load_configuration(struct conf *self) {
    FILE *conf_file;

    conf_file = fopen(CONFIGURATION_FILE_PATH, "r");
    /*Checks for -1*/
    if (conf_file == NULL) return -1;
    if (fscanf(conf_file, "so_user_num=%u", &self->so_user_num) == EOF) return -1;
    if (fscanf(conf_file, "so_nodes_num=%u", &self->so_nodes_num) == EOF) return -1;
    if (fscanf(conf_file, "so_budget_init=%u", &self->so_buget_init) == EOF) return -1;
    if (fscanf(conf_file, "so_reward=%f", &self->so_reward) == EOF) return -1;
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


/**
 * Load and read the configuration, in case of error during loading close the proc. with EXIT_FAILURE
 * @return TRUE if ALL OK
 */
Bool read_conf(struct conf simulation_conf) {

    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION...");
    switch (load_configuration(&simulation_conf)) {
        case 0:
            break;
        case -1:
            ERROR_EXIT_SEQUENCE(" during conf. loading: Missing File or Empty");
        case -2:
            ERROR_EXIT_SEQUENCE(" during conf. loading: Broken simultation logic, check conf. value");
        case -3:
            ERROR_EXIT_SEQUENCE(" during conf. loading: Not enough users for nodes");
        case -4:
            ERROR_EXIT_SEQUENCE(" during conf. loading: Min Max Execution time wrong");
        case -5:
            ERROR_EXIT_SEQUENCE(" during conf. loading: Node reward is over possibilities of users");
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CONFIGURATION LOADED");
    return TRUE;
}
