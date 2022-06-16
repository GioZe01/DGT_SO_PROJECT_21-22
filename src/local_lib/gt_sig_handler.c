/* Standard Library */
#include <stdio.h>
#include <stdlib.h>

/* Local Import */
#include "headers/gt_sig_handler.h"

void gen_mask(sigset_t *mask, int *sig_list, int sig_size) {
    sigemptyset(mask);
    int i;
    for (i = 0; i < sig_size; i++) {
        sigaddset(mask, sig_list[i]);
    }
}

Bool block_signals(sigset_t *new_mask, sigset_t *old_mask) {
    if (sigprocmask(SIG_BLOCK, new_mask, old_mask) < 0) {
        return FALSE;
    }
    return TRUE;
}

Bool unblock_signals(sigset_t *old_mask) {
    if (sigprocmask(SIG_SETMASK, old_mask, NULL) < 0) {
        return FALSE;
    }
    return TRUE;
}