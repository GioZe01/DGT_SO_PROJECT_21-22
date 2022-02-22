#ifndef DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H
#define DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H
#define PROC_TYPE_USER 1
#define PROC_TYPE_NODE 2
#define PROC_INFO_STATE_RUNNING 1
#define PROC_INFO_STATE_TERMINATED 0

#include <unistd.h>
#include <sys/types.h>

#include "boolean.h"
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
 * @return FALSE in case of failure. TRUE otherwise
 */
Bool send_sig_to_all(ProcList proc_list,int signal);
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
 */
void saving_private_ryan(ProcList self);
/*TODO: implementare budget maggiore get -> vedere se fare una hash table di processi*/
#endif /*DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H*/
