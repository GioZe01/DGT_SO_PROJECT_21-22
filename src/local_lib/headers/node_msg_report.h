/**
 * \file node_msg_report.h
 * \brief Header file for node_msg_report.c
 * Contains the definition of the functions and structures used to handle the node_msg_report message.
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_NODE_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_NODE_MSG_REPORT_H

#include <unistd.h>
#include "transaction_list.h"

/**
 * \defgroup NODE MSG TYPES
 * \brief MSG TYPES used by the node_msg_report message
 */
/** @{ */
#define DELTA_NODE_MSG_TYPE 3/**<To give priority to sigle node*/
#define MSG_MASTER_ORIGIN_ID 1/**< Contains the new friend id to be added in a binary form*/
#define MSG_NODE_ORIGIN_TYPE 2/**<For message generated by the node it'self */
#define MSG_TRANSACTION_TYPE 3/**<DEFAULT value */
/** @} */

/**
 * \struct node_msg
 * \brief Message structure
 */
struct node_msg {
    long type; /**< Type of the message */
    pid_t sender_pid; /**< PID of the sender */
    struct Transaction t; /**< Transaction structure */
};

/**
 *
 * Create the message given as param as address to a user_msg struct based on the given parameter
 * @param self give the add of your struct
 * @param type type of message to be send, has to be one of the defined in this .h
 * @param sender_pid pid of the current process how's calling this function
 * @param message_type type of message to be send, has to be one of the defined in this .h
 * @return 0 if success. -1 in case of failure
 */
int node_msg_create(struct node_msg *self, long type, pid_t sender_pid, struct Transaction *t, int message_type);

/**
 * Print the msg based on his type
 * @param self the message to be printed
 */
void node_msg_print(struct node_msg *self);

/**
* (Create if option selected) and Send the message on ipc user_queue with the key as specified
 * @warning send is repeated until it succeed or errno != EINTR or so_retry is not reached
 * @param id of the message queue
 * @param msg the message to be sent to
 * @param type type of message to be send, has to be one of the defined in this .h
 * @param t the transaction to be sent
 * @param sender pid_t of the sender of this message
 * @param crete TRUE if u want to create it before sending.
 * @param so_retry number of retry before failure occures.
 * @param queue_id number rappresenting the node id into the queue
 * @return -1 in case of failure, 0 otherwise
 */
int node_msg_snd(int id, struct node_msg *msg, long type, struct Transaction *t, pid_t sender, Bool crete, int so_retry,
                 int queue_id);

/**
 * Retrieve the message_user_report on the specified message queue
 * @warning the retriving operation is repeated until it succeed or errno != EINTR
 * @param id of the message queue
 * @param msg to be caught
 * @param type of message expected
 * @return -2 if no message on queue, -1 in case of failure. 0 otherwise.
 */
int node_msg_receive(int id, struct node_msg *msg, long type);

#endif /*DGT_SO_PROJECT_21_22_NODE_MSG_REPORT_H*/
