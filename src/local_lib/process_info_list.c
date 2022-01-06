#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "headers/glob_vars.h"
#include "headers/process_info_list.h"
#ifdef DEBUG
#include "headers/debug_utility.h"
#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

/*  Helper Funciton*/
void process_info_print(struct processes_info_list *pList);

/*  PORCESS TYPES   */
struct UserProcInfo {
    /*TODO: additional information based on the procces type*/
};

struct NodeProcInfo {
    /*TODO: additional information based on the procces type*/
} ;


struct processes_info_list *insert_in_list(struct processes_info_list *self, pid_t pid, short int type) {
    struct processes_info_list *new;
    new = (struct processes_info_list *) malloc(sizeof(*new));
    new->pid = pid;
    new->proc_type = type;
    new->proc_state = PROC_INFO_STATE_RUNNING;
    new->next = self;
}

struct processes_info_list *get_proc_from_pid(struct processes_info_list *self, pid_t pid) {
    /*Implemented linear search :( */
    for (; self != NULL; self = self->next)
        if (self->pid == pid) return self;
    return NULL;
}


void print_list(struct processes_info_list *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("PRINTING PROCESS LIST...");
    for (; self != NULL; self = self->next) {
        process_info_print(self);
        if (self->next != NULL) printf("\n");
    }
    printf("\n");
    DEBUG_NOTIFY_ACTIVITY_DONE("PRINTING PROCESS LIST ENDED");
}

void process_info_print(struct processes_info_list *pList) {
    if (pList != NULL)
        printf("# pid : %d | proc_type : %s | proc_state : %s | budget : %f ",
               pList->pid,
               pList->proc_type == PROC_TYPE_USER ? "User" : "Node",
               pList->proc_state == PROC_INFO_STATE_RUNNING ? "Running" : "Terminated",
               pList->budget
        );
    else
            DEBUG_ERROR_MESSAGE("Calling print_list on NULL process info");
}

void list_free(struct processes_info_list *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("PROCESSES INFO LIST FREE...");
    /*Recursive implementation*/
    if (self == NULL){
        DEBUG_MESSAGE("LIST OF PROC WAS NULL");
        return;
    }
    list_free(self->next);
    free(self);
    DEBUG_NOTIFY_ACTIVITY_DONE("PROCESSES INFO LIST FEE DONE");
}

void list_set_state(struct processes_info_list *self, pid_t pid, short int state) {
    /*linear search implemented*/
    for (; self != NULL; self = self->next)
        if (self->pid == pid) self->proc_state = state;
}
