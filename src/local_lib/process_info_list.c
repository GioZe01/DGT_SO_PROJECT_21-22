#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "headers/glob_vars.h"
#include "headers/process_info_list.h"
#include "headers/debug_utility.h"

/*  Helper Funciton*/
void process_info_print(struct processses_info_list *pList);

/*  PORCESS TYPES   */
typedef struct {

} UserProcInfo;

typedef struct {

} NodeProcInfo;
/*  PROCESS TYPE DEFINITION */
union ProcInfo {
    UserProcInfo;
    NodeProcInfo;
};

struct processses_info_list *insert(struct processses_info_list *self, pid_t pid, short int type, ProcInfo proc_info) {
    struct processses_info_list *new;
    new = (struct processses_info_list *) malloc(sizeof(*new));
    new->pid = pid;
    new->proc_type = type;
    new->proc_state = PROC_INFO_STATE_RUNNING;
    new->proc = proc_info;
    new->next = self;
}

struct processses_info_list *get_proc_from_pid(struct processses_info_list *self, pid_t pid) {
    /*Implemented linear search :( */
    for (; self != NULL; self = self->next)
        if (self->pid == pid) return self;
    return NULL;
}


void print(struct processses_info_list *self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("PRINTING PROCESS LIST...");
    for (; self != NULL; self = self->next) {
        process_info_print(self);
        if (self->next != NULL) printf("\n");
    }
    printf("\n");
    DEBUG_NOTIFY_ACTIVITY_DONE("PRINTING PROCESS LIST ENDED");
}

void process_info_print(struct processses_info_list *pList) {
    /*TODO: implement printing*/
}

void list_free(struct processses_info_list *self) {
    /*Recursive implementation*/
    if (self == NULL) return;
    list_free(self->next);
    free(self);
}

void list_set_state(struct processses_info_list *self, pid_t pid, short int state) {
    /*linear search implemented*/
    for (; self != NULL; self = self->next)
        if (self->pid == pid) self->proc_state = state;
}