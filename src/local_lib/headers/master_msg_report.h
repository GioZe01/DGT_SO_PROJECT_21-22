
#ifndef DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H
/* Std */
#include <unistd.h>

/* Local */
#include "boolean.h"
#include "process_info_list.h"
#include "transaction_list.h"

typedef enum {
    PROC_STATE_INIT, PROC_STATE_RUNNING,PROC_STATE_TERMINATED,PROC_STATE_WAITING, PROC_STATE_NODE_SERV_TRANS
}PROCSTATE;
typedef enum {
    TERMINATION_END_CORRECTLY = 1,
    IMPOSSIBLE_TO_SEND_TRANSACTION = 2,
    IMPOSSIBLE_TO_CONNECT_TO_SHM = 3,
    IMPOSSIBLE_TO_COMUNICATE_WITH_QUEUE = 4,
    SIGNALS_OF_TERM_RECEIVED = 5,
    INFO_BUDGET = 6,
    TP_FULL = 7,
    UNUSED_PROC = 8,
}MSG_REPORT_TYPE;

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
    struct Transaction t;
};

/**
 * Constructor for the message to be sent to the master_proc
 * @param self the message
 * @param type of the message, see MSG_REPORT_ABORTED_PROC as exmp.
 * @param sender_pid the pid of the user to sent this msg
 * @param state state of the current proc who's sending this message
 * @param budget (Optional -1 as value), set if type is INFO_BUDGET otherwise budget set to 0
 * @param t set if type is TP_FULL otherwise t set to empty
 */
void
master_msg_report_create(struct master_msg_report *self, long type, long proc_type, pid_t sender_pid, short int state, float budget, struct Transaction t );

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
 * @param t set if type is TP_FULL otherwise t set to empty
 * @return -1 in case of failure. 0 otherwise
 * */
int
master_msg_send(int id, struct master_msg_report *self, long type, long proc_type, pid_t sender_pid, short int state,
                Bool create, float budget, struct Transaction* t);

/**
 * Retrive the message_node_report on the specified message queue
 * @warning the retriving operation is repeated until it succeed or errno != EINTR
 * @param id of the message queue
 * @param msg to be caught
 * @param type of the message, see MSG_REPORT_ABORTED_PROC as exmp.
 * @return -2 in case of no msg on queue. -1 in case of failure. 0 otherwise
 */
int master_msg_receive(int id, struct master_msg_report *self);

/**
 * Check for messages in the master msg_queue
 * @param msg_report pointer to the struct representing the msg in the master msg queue
 * @param msg_report_id_master id of the queue
 * @param proc_list list of the process to check msg for
 * @return -1 in case of FAILURE. 0 otherwise
 */
int check_msg_report(struct master_msg_report *msg_report, int msg_report_id_master, ProcList proc_list);

/**
 * Retrive the message_node_report on the specified message queue
 * @warning the retriving operation is repeated until it succeed or errno != EINTR
 * @param id of the message queue
 * @param msg to be caught
 * @return -2 in case of no msg on queue. -1 in case of failure. 0 otherwise
 */
int master_msg_receive_info(int id, struct master_msg_report *self);

/**
 * Based on the given master msg, load and modify the data in proc list
 * @param self the message
 * @param list the list of process to be updated
 * @return -2 in case of no sender pid in proc_list.-1 in case of wrong msg_type.1 in case of TP_FULL and  0 otherwise.
 * TODO: Implement termination info in the list of proc (Cause that determined the termination)
 **/
int acknowledge(struct master_msg_report * self, ProcList list);

/**
 * Convert the given type into his relative string
 * @param type the type of the message
 * @return the string equivalent
 */
char * from_type_to_string(long type);

/**
 * Convert the given exec type into his equivalent string
 * @param state current state of the proc
 * @param string var in wich the method save the equivalent string
 */
void from_procstate_to_string(int state, char * string);

/**
 * Convert the given proc_type into his relative string
 * @param proc_type the process type to be converted
 * @return the equivalent string
 */
char * from_proctype_to_string(long proc_type);
#endif /*DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H*/
