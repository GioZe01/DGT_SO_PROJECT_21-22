#ifndef DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H
#define DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H

#include <unistd.h>
#include <sys/types.h>

#include "boolean.h"

typedef enum {
    PROC_TYPE_USER, PROC_TYPE_NODE,
} PROCTYPE;

struct ProcessInfo {
    pid_t pid;
    int id_queue;/*Id for calculating range type of a user*/
    short int proc_state;
    short int proc_type;
    float budget;/*TODO: verificare se basta*/
};
typedef struct ProcessInfo *Proc;
typedef struct processes_info_list *ProcList;

/**
 * Create the ProcList Linked list
 * @param list the list to be created
 * @return FALSE if the list is not created, TRUE otherwise
 */
ProcList proc_list_create();

/**
 * Insert the new process and initialize it in running mode
 * @param self the next of the list to be linked with
 * @param pid the pid of the new process
 * @param id_queue the id of the queue of the new process
 * @param proc_type the type of the new process
 * @param budget of the new process
 * @param proc_state the state of the new process
 * @return FALSE if the process is not inserted, TRUE otherwise
 */
Bool insert_in_list(ProcList *self, pid_t pid, int id_queue, short int proc_state, short int proc_type, float budget);

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
struct ProcessInfo get_proc_from_pid(ProcList self, pid_t pid);

/**
 * Return the process inside the list associated with the id_queue given as param
 * @param self the list to search in
 * @param queue_id the value to search for
 * @return the element with the specified pid if found. Otherwise NULL
 */
struct ProcessInfo get_proc_from_queue_id(ProcList self, int id_queue);

/**
 * \brief Send the given signal to all proc saved via kill() method and save in pids the list of pid of the processes killed
 *
 * @param proc_list the list of proced to receive the signal
 * @param signal signal to be sent
 * @param pids the list where the pid of the killed processes will be saved
 * @warning in the 0 index of the pids list is saved the number of processes killed
 */
void send_sig_to_all(ProcList proc_list, int signal, int *pids);

/**
 * Send the given signal to the process with type node
 * @param proc_list the list of proced to receive the signal
 * @param signal signal to be sent
 * @param exclude_last if TRUE the last node will not receive the signal
 * @return -1 in case of failure. the number of proc to wait responce for
 */
int send_sig_to_all_nodes(ProcList proc_list, int signal, Bool exclude_last);

/**
 * Print at console the specified list if the list is too big (MAX_PROC_TO_PRINT into glob.h) it will print the processes with the highest budget
 * @param self ref of the list to print_list
 * @warning the list is sorted by budget
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
 * @brief Return the number of proc in the list
 * @param self the list to search in
 */
int get_num_of_proc(ProcList self);

/**
 * Return the number of active user_proc
 * @return integer rappresenting the number of active user
 */
int get_num_of_user_proc_running(ProcList self);

/**
 * Return the number of active node_proc
 * @return integer rappresenting the number of active user
 */
int get_num_of_node_proc_running(ProcList self);

/**
 * @brief send a msg with the new node id to all the other nodes
 * @param queue_id id of the queue to snd the msg on
 * @param retry number of retry to send the single msg
 * @param node_id the new node id
 * @exclude_last exclude the last node from the list
 * @return FALSE in case of FAILURE, TRUE otherwise
 */
Bool send_msg_to_all_nodes(int queue_id, int retry, ProcList proc_list, int node_id, Bool exclude_last);

/**
 * @brief Extract randomly a num_of_node from the proc_list and insert them into the node_list
 * @param proc_list the list of proc to extract from
 * @param node_list the list of node to insert into
 * @param num_of_node the number of node to extract
 * @warning if the number of nodes into the proc_list is less than the num_of_node, the node_list will be equal to the proc_list
 */
void get_random_node_list(ProcList proc_list, ProcList node_list, int num_of_node);

/**
 * @brief Get the first proc in the list
 * @param self the list to search in
 * @return The first proc in the list
 */
struct ProcessInfo get_first(ProcList self);

/**
 * @brief Update if found the proc with the given pid in the list
 * @param self the list to search in
 * @param pid  the pid to search for
 * @param budget  the new budget
 * @param proc_state  the new proc_state
 * @return -2 if the pid is not found, -1 if the budget is not valid, 0 otherwise
 */
int update_proc(ProcList self, int pid, float budget, short int proc_state);

/**
 * @brief Get the running node in the list
 * @param self the list to search in
 * @return The running nods list
 */
ProcList get_running_node_proc(ProcList self);

/**
 * @brief Get the num of proc in running state
 * @param self the list to search in
 * @return the number of proc in running state
 */
int get_num_proc_running(ProcList self);
/*TODO: implementare budget maggiore get -> vedere se fare una hash table di processi*/
#endif /*DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H*/
