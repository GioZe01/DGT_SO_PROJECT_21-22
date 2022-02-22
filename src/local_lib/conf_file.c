
/* Std */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* Local*/
#include "headers/conf_file.h"

#ifdef DEBUG

#include "headers/debug_utility.h"

#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#define DEBUG_BLOCK_ACTION_START (mex)
#define DEBUG_BLOCK_ACTION_END (mex)
#endif

#include "headers/simulation_errors.h"
#include "headers/boolean.h"

int load_configuration(struct conf *self) {
    FILE *conf_file;
    DEBUG_NOTIFY_ACTIVITY_RUNNING("LOADING CONFIGURATION.... ");
    conf_file = fopen(CONFIGURATION_FILE_PATH, "r");
    /*Checks for -1*/
    if (conf_file == NULL) {
        return -1;
    }
    if (fscanf(conf_file, "so_user_num=%d ", &self->so_user_num) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_nodes_num=%d ", &self->so_nodes_num) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_budget_init=%f ", &self->so_buget_init) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_reward=%d ", &self->so_reward) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_min_trans_gen_nsec=%ld ", &self->so_min_trans_gen_nsec) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_max_trans_gen_nsec=%ld ", &self->so_max_trans_gen_nsec) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_retry=%d ", &self->so_retry) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_tp_size=%d ", &self->so_tp_size) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_min_trans_proc_nsec=%ld ", &self->so_min_trans_proc_nsec) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_max_trans_proc_nsec=%ld ", &self->so_max_trans_proc_nsec) == EOF) {
        fclose(conf_file);
        return -1;
    }
    if (fscanf(conf_file, "so_sim_sec=%ld ", &self->so_sim_sec) == EOF) {
        fclose(conf_file);
        return -1;
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("LOADING CONFIGURATION DONE");
    DEBUG_NOTIFY_ACTIVITY_RUNNING("CHECKING CONF VALUE....");
    /*Checks for -2*/
    if (self->so_user_num <= 0 ||
        self->so_nodes_num <= 0 ||
        self->so_buget_init <= 0 ||
        self->so_reward < 0 ||
        self->so_min_trans_gen_nsec <= 0 ||
        self->so_max_trans_gen_nsec <= 0 ||
        self->so_retry < 0 ||
        self->so_tp_size <= 0 ||
        self->so_min_trans_proc_nsec <= 0 ||
        self->so_max_trans_proc_nsec <= 0 ||
        self->so_sim_sec <= 0){
        fclose(conf_file);
        return -2;}
    /*Checks for -3*/
    if (self->so_user_num < 2 || self->so_nodes_num < 1) {/*TODO: numbers nodes check requested ?*/
        fclose(conf_file);
        return -3;
    }
    /*Checks for -4*/
    if (self->so_min_trans_proc_nsec > self->so_max_trans_proc_nsec ||
        self->so_min_trans_gen_nsec > self->so_max_trans_gen_nsec) {
        fclose(conf_file);
        return -4;
    }
    if (self->so_reward > 1) { /*Vorrebbe dire che con una transazione da max budget non puoi pagare le spese*/
        fclose(conf_file);
        return -5;
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("CHECKING CONF VALUE DONE");
    fclose(conf_file);
    return 0;
}


