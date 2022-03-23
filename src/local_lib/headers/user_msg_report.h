#ifndef DGT_SO_PROJECT_21_22_USER_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_USER_MSG_REPORT_H

#include <unistd.h>
#include "transaction_list.h"
#include "boolean.h"

#define DELTA_USER_MSG_TYPE 3/*To give priority to single user*/
#define MSG_TRANSACTION_FAILED_TYPE 1
#define MSG_TRANSACTION_INCOME_TYPE 2
#define MSG_TRANSACTION_CONFIRMED_TYPE 3/*Default value*/

#define CHECK_USER_TYPE(type_recived, queue_id) type_recived-queue_id == 0 ? MSG_TRANSACTION_CONFIRMED_TYPE : type_recived

struct user_msg {
    long type;
    pid_t sender_pid;
    struct Transaction t;
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
int user_msg_create(struct user_msg *self, long type, pid_t sender_pid, struct Transaction *t);

/**
 * Print the msg based on his type
 * @param self the message to be printed
 */
void user_msg_print(struct user_msg *self);

/**
* (Create if selected) and Send the message on ipc user_queue with the key as specified
 * @param id of the message queue
 * @param msg the message to be sent to
 * @param data user_data to be sent
 * @param sender pid_t of the sender of this message
 * @param crete TRUE if u want to create it before sending.
 * @param queue_id number rappresenting the current user into the message queue
 * @return -1 in case of failure, -1 otherwise
 */
int user_msg_snd(int id, struct user_msg *msg, long type, struct Transaction *t, pid_t sender, Bool crete, int queue_id);

/**
 * Retrieve the message_user_report on the specified message queue
 * @warning send is repeated until it succeed or errno != EINTR
 * Config type wait until receive
 * @param id of the message queue
 * @param msg to be caught
 * @param type of message expected
 * @return -2 if no message on queue, -1 in case of failure. 0 otherwise.
 */
int user_msg_receive(int id, struct user_msg *msg, long type);

#endif /*DGT_SO_PROJECT_21_22_USER_MSG_REPORT_H*/
