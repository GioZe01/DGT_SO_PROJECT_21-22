
#ifndef DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H
#define DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H
#include <unistd.h>
#define PROC_STATE_INIT 0
#define PROC_STATE_RUNNING 1
#define PROC_STATE_WAITING 2
#define PROC_STATE_NODE_SERV_TRANS 3
/*  ABORT TYPE */
typedef enum {
    IMPOSSIBLE_TO_SEND_TRANSACTION,
    IMPOSSIBLE_TO_CONNECT_TO_SHM,

}MSG_REPORT_ABORTED_PROC;

struct master_msg_report{
    long type;
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
void master_msg_report_create(struct master_msg_report * self,long type, pid_t sender_pid, short int state);
void master_msg_report_print(struct master_msg_report * self);
#endif /*DGT_SO_PROJECT_21_22_MASTER_MSG_REPORT_H*/
