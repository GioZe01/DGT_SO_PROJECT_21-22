#ifndef DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H
#define DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H
#define PROC_TYPE_USER 1
#define PROC_TYPE_NODE 2
#define PROC_INFO_STATE_RUNNING 1
#define PROC_INFO_STATE_TERMINATED 0

#include <unistd.h>
/*  PROCESS TYPE DEFINITION */
struct processes_info_list{
    pid_t pid;
    short int proc_state;
    short int proc_type;
    float budget;/*TODO: verificare se basta*/
    struct processes_info_list* next;
};
/**
 * Insert the new process and initialize it in running mode
 * @param self the next of the list to be linked with
 * @param pid process id of the proc to be rappresented int the struct list
 * @param type proc_type ex: PROC_TYPE_USER or PROC_TYPE_NODE
 * @return the new linked list with the structure inserted
 */
struct processes_info_list* insert_in_list(struct processes_info_list* self, pid_t pid, short int type);
/**
 * Return the process inside the list associated with the pid given as param
 * @param self the list to search in
 * @param pid the value to search for
 * @return the element with the specified pid if found. Otherwise NULL
 */
struct processes_info_list* get_proc_from_pid(struct processes_info_list* self, pid_t pid);
/**
 * Print at console the specified list
 * @param self ref of the list to print_list
 */
void print_list(struct processes_info_list* self);
/**
 * Free the memory from the list
 * @param self ref. to the list to free
 */
void list_free(struct processes_info_list* self);
/**
 * Change the flag state of a record in the list
 * @param self ref. of the list
 * @param pid  record identifier
 * @param state the proc_state to be set
 */
void list_set_state(struct processes_info_list* self, pid_t pid, short int state);
/*TODO: implementare budget maggiore get -> vedere se fare una hash table di processi*/
#endif /*DGT_SO_PROJECT_21_22_PROCESS_INFO_LIST_H*/
