/**
 * \file gt_sig_handler.h
 * \brief Header for gt_sig_handler.c
 * Define the sig handler function prototype
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_GT_SIG_HANDLER_H
#define DGT_SO_PROJECT_21_22_GT_SIG_HANDLER_H
/* Standard includes */
#include <signal.h>
/* Local includes */
#include "boolean.h"

/**
 * @brief Generate a mask for the signals to be blocked
 * @param mask the mask to be generated
 * @param sig_list the list of signals to be blocked
 * @param sig_size the size of the list of signals
 */
void gen_mask(sigset_t *mask, int *sig_list, int sig_size);

/**
 * @brief Block the signals of the new mask
 * @param new_mask the new mask to block
 * @param old_mask pointer to the old mask
 * @return TRUE if success, FALSE otherwise
 */
Bool block_signals(sigset_t *new_mask, sigset_t *old_mask);

/**
 * @brief Set the old mask given
 * @param old_mask pointer to the old mask
 * @return TRUE if success, FALSE otherwise
 */
Bool unblock_signals(sigset_t *old_mask);

#endif /*DGT_SO_PROJECT_21_22_GT_SIG_HANDLER_H*/
