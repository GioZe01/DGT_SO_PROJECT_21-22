
#ifndef DGT_SO_PROJECT_21_22_NODE_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_NODE_MSG_REPORT_H
#define MSG_CONFIG_TYPE 1
#define MSG_NODE_ORIGIN_TYPE 2 /* For message generated by the node it'self */
#define MSG_TRANSACTION_TYPE 3
#include <unistd.h>
#include "transaction_list.h"

typedef union {
    struct Transaction t;
    int *nodes_id_to_pid;
} node_data;
struct node_msg {
    long type;
    pid_t sender_pid;
    node_data data;
};

/**
 *
 * Create the message given as param as address to a user_msg struct based on the given parameter
 * @param self give the add of your struct
 * @param type type of message to be send, has to be one of the defined in this .h
 * @param sender_pid pid of the current process how's calling this function
 * @param data user_data to send
 * @return 0 if success. -1 in case of failure
 */
int node_msg_create(struct node_msg *self, long type, pid_t sender_pid, node_data *data);

/**
 * Print the msg based on his type
 * @param self the message to be printed
 */
void node_msg_print(struct node_msg *self);

/**
* (Create if selected) and Send the message on ipc user_queue with the key as specified
 * @param id of the message queue
 * @param msg the message to be sent to
 * @param data user_data to be sent
 * @param sender pid_t of the sender of this message
 * @param crete TRUE if u want to create it before sending.
 * @return -1 in case of failure, -1 otherwise
 */
int node_msg_snd(int id, struct node_msg *msg, long type, node_data *data, pid_t sender, Bool crete);

/**
 * Retrieve the message_user_report on the specified message queue
 * @warning send is repeated until it succeed or errno != EINTR
 * Config type wait until receive
 * @param id of the message queue
 * @param msg to be caught
 * @param type of message expected
 * @return -2 if no message on queue, -1 in case of failure. 0 otherwise.
 */
int node_msg_receive(int id, struct node_msg *msg, long type);

#endif /*DGT_SO_PROJECT_21_22_NODE_MSG_REPORT_H*/
