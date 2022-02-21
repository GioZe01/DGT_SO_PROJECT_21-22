
#ifndef DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H

#include <unistd.h>
#include "boolean.h"

#define PROC_STATE_INIT 0
#define PROC_STATE_RUNNING 1
#define PROC_STATE_WAITING 2
#define PROC_STATE_NODE_SERV_TRANS 3

/*  ABORT TYPE */
typedef enum {
    TERMINATION_END_CORRECTLY,
    IMPOSSIBLE_TO_SEND_TRANSACTION,
    IMPOSSIBLE_TO_CONNECT_TO_SHM,
    SIGNALS_OF_TERM_RECEIVED,
}MSG_REPORT_TYPE;
typedef enum{
    USER,
    NODE,
    NODE_TP,
}PROC_TYPE;
struct master_msg_report{
    long type;
    long proc_type;
    pid_t sender_pid;
    short int state;
};
/**
 * Constructor for the message to be sent to the master_proc
 * @param self the message
 * @param type of the message, see MSG_REPORT_ABORTED_PROC as exmp.
 * @param sender_pid the pid of the user to sent this msg
 * @param state state of the current proc who's sending this message
 */
void master_msg_report_create(struct master_msg_report * self,long type, long proc_type, pid_t sender_pid, short int state);

/**
 * Print the message based on his type
 * @param sel the messagge to be printed
 * */
void master_msg_report_print(struct master_msg_report * self);

/**
 * (Create if option selected) and Send the message on ipc master_msg_queue with the given  key
 * @warning send is repeated until it succeed or errno != EINTR
 * @param self the message
 * @param type of the message, see MSG_REPORT_ABORTED_PROC as exmp.
 * @param sender_pid the pid of the user to sent this msg
 * @param state state of the current proc who's sending this message
 * @return -1 in case of failure. 0 otherwise
 * */
int master_msg_send(int id, struct master_msg_report * self,long type,long proc_type, pid_t sender_pid, short int state, Bool create);

/**
 * Retrive the message_node_report on the specified message queue
 * @warning the retriving operation is repeated until it succeed or errno != EINTR
 * @param id of the message queue
 * @param msg to be caught
 * @return -2 in case of no msg on queue. -1 in case of failure. 0 otherwise
 */
int master_msg_receive(int id, struct master_msg_report * self);
#endif /*DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H*/
