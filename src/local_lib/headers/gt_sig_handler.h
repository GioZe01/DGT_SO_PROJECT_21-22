#ifndef DGT_SO_PROJECT_21_22_GT_SIG_HANDLER_H
#define DGT_SO_PROJECT_21_22_GT_SIG_HANDLER_H
/* Standard includes */
#include <signal.h>

/**
 * @brief Block the given list of signal. If the signal is already blocked, do nothing.
 * @param sigs_num The signals to be block.
 * @param old_sig_mask The old signal mask.
 */
void block_signals(int* sigs_num, sigset_t *old_sig_mask);
/**
 * @brief Unblock the given list of signal. If the signal is already unblocked, do nothing.
 * @param sig_num The signals to be unblock.
 * @param old_sig_mask The old signal mask.
 */
void unblock_signals(int * sigs_num, sigset_t *old_sig_mask);
#endif /*DGT_SO_PROJECT_21_22_GT_SIG_HANDLER_H*/
