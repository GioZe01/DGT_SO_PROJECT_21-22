
#ifndef DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H

#include <unistd.h>
#include "boolean.h"

typedef enum {
    PROC_STATE_INIT, PROC_STATE_RUNNING,PROC_STATE_TERMINATED,PROC_STATE_WAITING, PROC_STATE_NODE_SERV_TRANS
} PROCSTATE;
/*  ABORT TYPE */
typedef enum {
    TERMINATION_END_CORRECTLY,
    IMPOSSIBLE_TO_SEND_TRANSACTION,
    IMPOSSIBLE_TO_CONNECT_TO_SHM,
    SIGNALS_OF_TERM_RECEIVED,
    INFO_BUDGET,
} MSG_REPORT_TYPE;
typedef enum {
    USER,
    NODE,
    NODE_TP,
} PROC_TYPE;
struct master_msg_report {
    long type;
    long proc_type;
    pid_t sender_pid;
    short int state;
    float budget;
};

/**
 * Constructor for the message to be sent to the master_proc
 * @param self the message
 * @param type of the message, see MSG_REPORT_ABORTED_PROC as exmp.
 * @param sender_pid the pid of the user to sent this msg
 * @param state state of the current proc who's sending this message
 * @param budget (Optional -1 as value), set if type is INFO_BUDGET otherwise budget set to 0
 */
void
master_msg_report_create(struct master_msg_report *self, long type, long proc_type, pid_t sender_pid, short int state, float budget);

/**
 * Print the message based on his type
 * @param sel the messagge to be printed
 * */
void master_msg_report_print(const struct master_msg_report *self);

/**
 * (Create if option selected) and Send the message on ipc master_msg_queue with the given  key
 * @warning send is repeated until it succeed or errno != EINTR
 * @param self the message
 * @param type of the message, see MSG_REPORT_ABORTED_PROC as exmp.
 * @param sender_pid the pid of the user to sent this msg
 * @param state state of the current proc who's sending this message
 * @param budget (Optional -1 as value), set if type is INFO_BUDGET otherwise budget set to 0
 * @return -1 in case of failure. 0 otherwise
 * */
int
master_msg_send(int id, struct master_msg_report *self, long type, long proc_type, pid_t sender_pid, short int state,
                Bool create, float budget);

/**
 * Retrive the message_node_report on the specified message queue
 * @warning the retriving operation is repeated until it succeed or errno != EINTR
 * @param id of the message queue
 * @param msg to be caught
 * @return -2 in case of no msg on queue. -1 in case of failure. 0 otherwise
 */
int master_msg_receive(int id, struct master_msg_report *self);

/**
 * Retrive the message_node_report on the specified message queue
 * @warning the retriving operation is repeated until it succeed or errno != EINTR
 * @param id of the message queue
 * @param msg to be caught
 * @return -2 in case of no msg on queue. -1 in case of failure. 0 otherwise
 */
int master_msg_receive_info(int id, struct master_msg_report *self);

#endif /*DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H*/
