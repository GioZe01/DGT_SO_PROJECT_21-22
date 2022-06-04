/* Standard Library */
#include <stdio.h>
#include <stdlib.h>

/* Local Import */
#include "headers/gt_sig_handler.h"
#include "headers/simulation_errors.h"

void block_signal(int sig_num, sigset_t *old_sig_mask) {
    sigset_t new_sig_mask;
    sigemptyset(&new_sig_mask);
    sigaddset(&new_sig_mask, sig_num);
    if (sigprocmask(SIG_BLOCK, &new_sig_mask, old_sig_mask) == -1) {
        ERROR_MESSAGE("Error blocking signal");
    }
}

void unblock_signal(int sig_num, sigset_t *old_sig_mask) {
    sigset_t new_sig_mask;
    sigemptyset(&new_sig_mask);
    sigaddset(&new_sig_mask, sig_num);
    if (sigprocmask(SIG_UNBLOCK, &new_sig_mask, old_sig_mask) == -1) {
        ERROR_MESSAGE("Error unblocking signal");
    }
}