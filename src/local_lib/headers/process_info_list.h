#ifndef DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H
#define DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H

#include <unistd.h>
#include <sys/types.h>

#include "boolean.h"

typedef enum {
    PROC_TYPE_USER, PROC_TYPE_NODE,
} PROCTYPE;

struct ProcessInfo{
    pid_t pid;
    int id_queue;/*Id for calculating range type of a user*/
    short int proc_state;
    short int proc_type;
    float budget;/*TODO: verificare se basta*/
};
typedef struct ProcessInfo * Proc;
typedef struct processes_info_list * ProcList;

/**
 * Create the ProcList Linked list
 * @return the linked list created
 * */
ProcList proc_list_create();
/**
 * Insert the new process and initialize it in running mode
 * @param self the next of the list to be linked with
 * @param pid process id of the proc to be rappresented int the struct list
 * @param type proc_type ex: PROC_TYPE_USER or PROC_TYPE_NODE
 * @return the new linked list with the structure inserted
 */
void insert_in_list(ProcList self, pid_t pid, short int type, int id_queue);
/**
 * Check if the ProcList is empty
 * @return TRUE if is empty, FALSE otherwise
 * */
Bool proc_list_is_empty(const ProcList self);
/**
 * Return the process inside the list associated with the pid given as param
 * @param self the list to search in
 * @param pid the value to search for
 * @return the element with the specified pid if found. Otherwise NULL
 */
Proc get_proc_from_pid(ProcList self, pid_t pid);
/**
 * Return the process inside the list associated with the id_queue given as param
 * @param self the list to search in
 * @param queue_id the value to search for
 * @return the element with the specified pid if found. Otherwise NULL
 */
Proc get_proc_from_queue_id(ProcList self, int id_queue);
/**
 * Send the given signal to all proc saved via kill() method
 * @param proc_list the list of proced to receive the signal
 * @param signal signal to be sent
 * @return -1 in case of failure. the number of proc to wait responce for
 */
int send_sig_to_all(ProcList proc_list,int signal);

/**
 * Send the given signal to the process with type node
 * @param proc_list the list of proced to receive the signal
 * @param signal signal to be sent
 * @param exclude_last if TRUE the last node will not receive the signal
 * @return -1 in case of failure. the number of proc to wait responce for
 */
int send_sig_to_all_nodes(ProcList proc_list,int signal, Bool exclude_last);

/**
 * Print at console the specified list
 * @param self ref of the list to print_list
 */
void print_list(ProcList self);
/**
 * Free the memory from the list
 * @param self ref. to the list to free
 */
void list_free(ProcList self);
/**
 * Change the flag state of a record in the list
 * @param self ref. of the list
 * @param pid  record identifier
 * @param state the proc_state to be set
 */
void list_set_state(ProcList self, pid_t pid, short int state);

/**
 * Kill all the process in the list
 * @param self the list of proc to kill
 */
void terminator(ProcList self);

/**
 * Wait for all proc to terminate
 * @param self list of all the proc to wait for
 * @param queue_id id of the queue of master msg
 */
void saving_private_ryan(ProcList self, int queue_id);

/**
 * Return the number of active user_proc
 * @return integer rappresenting the number of active user
 */
int get_num_of_user_proc_running(ProcList self);


/**
 * @brief send a msg with the new node id to all the other nodes
 * @param queue_id id of the queue to snd the msg on
 * @param retry number of retry to send the single msg
 * @param node_id the new node id
 * @exclude_last exclude the last node from the list
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool send_msg_to_all_nodes(int queue_id,int retry,  ProcList proc_list, int node_id, Bool exclude_last);
/*TODO: implementare budget maggiore get -> vedere se fare una hash table di processi*/
#endif /*DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H*/
