/* Standard Library */
#include <stdio.h>
#include <stdlib.h>

/* Local Import */
#include "headers/gt_sig_handler.h"
#include "headers/simulation_errors.h"

void block_signals(int* sigs_num, sigset_t *old_sig_mask) {
    sigset_t new_sig_mask;
    sigemptyset(&new_sig_mask);
    while (sigs_num != NULL && *sigs_num != 0) {
        sigaddset(&new_sig_mask, *sigs_num);
        sigs_num++;
    }
    if (sigprocmask(SIG_BLOCK, &new_sig_mask, old_sig_mask) == -1) {
        ERROR_MESSAGE("Error blocking signal");
    }
}

void unblock_signals(int * sigs_num, sigset_t *old_sig_mask) {
    sigset_t new_sig_mask;
    sigemptyset(&new_sig_mask);
    while (sigs_num != NULL && *sigs_num != 0) {
        sigaddset(&new_sig_mask, *sigs_num);
        sigs_num++;
    }
    if (sigprocmask(SIG_UNBLOCK, &new_sig_mask, old_sig_mask) == -1) {
        ERROR_MESSAGE("Error unblocking signal");
    }
}