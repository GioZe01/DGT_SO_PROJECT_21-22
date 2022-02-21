#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include "headers/glob.h"
#include "headers/process_info_list.h"
#include "headers/simulation_errors.h"
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
void process_info_print(Proc p);
void proc_list_remove_head(ProcList self);
void proc_list_underflow();

struct node{
    Proc p;
    struct node* next;
};
struct processes_info_list{
    struct node * first;
    struct node * last;
    int num_proc;
};
ProcList proc_list_create(){
    ProcList p = malloc(sizeof(struct processes_info_list));
    p->first = NULL;
    p->last = NULL;
    p->num_proc = 0;
    return p;
}

void insert_in_list(struct processes_info_list *self, pid_t pid, short int type, int queue_id) {
    struct node * new;
    if ((new = malloc(sizeof(struct node)))==NULL){
        ERROR_MESSAGE("IMPOSSIBLE TO INSERT NEW PROC");
    }
    new->next = NULL;
    new->p->pid = pid;
    new->p->proc_type = type;
    new->p->proc_state = PROC_INFO_STATE_RUNNING;
    new->p->id_queue = queue_id;
    new->p->budget = -1;
    if (proc_list_is_empty(self) == TRUE){
        self->first = self->last;
    }
    else{
        self->last->next = new;
        self->last = new;
    }
   self->num_proc++;
}
Bool proc_list_is_empty(ProcList self){
    return (self->num_proc ==0) ? TRUE : FALSE;
}
Proc get_proc_from_queue_id(ProcList self, int id_queue){
    /*Implemented linear search :( */
    struct node * tmp = self->first;
    for (;tmp != NULL; tmp = tmp->next){
        if(tmp->p->id_queue == id_queue) return tmp->p;
    }
    return NULL;
}
Proc get_proc_from_pid(ProcList self, pid_t pid) {
    /*Implemented linear search :( */
    struct node * tmp = self->first;
    for (;tmp != NULL; tmp = tmp->next){
        if(tmp->p->pid== pid) return tmp->p;
    }
    return NULL;
}


void print_list(ProcList self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("PRINTING PROCESS LIST...");
    struct node * tmp = self->first;
    for (; tmp!= NULL; tmp= tmp->next){
        process_info_print(tmp->p);
        if (tmp->next != NULL) printf("\n");
    }
    printf("\n");
    DEBUG_NOTIFY_ACTIVITY_DONE("PRINTING PROCESS LIST ENDED");
}

void process_info_print(Proc p) {
    if (p!= NULL)
        printf("# pid : %d | proc_type : %s | proc_state : %s | budget : %f ",
               p->pid,
               p->proc_type == PROC_TYPE_USER ? "User" : "Node",
               p->proc_state == PROC_INFO_STATE_RUNNING ? "Running" : "Terminated",
               p->budget
        );
    else
            DEBUG_ERROR_MESSAGE("Calling print_list on NULL process info");
}

void list_free(ProcList self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("PROCESSES INFO LIST FREE...");
    while(proc_list_is_empty(self)==FALSE){
        proc_list_remove_head(self);
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("PROCESSES INFO LIST FREE DONE");
}

void list_set_state(ProcList self, pid_t pid, short int state) {
    /*linear search implemented*/
    struct node *tmp = self->first;
    for (;tmp != NULL; tmp = tmp->next){
        struct node * tmp = self->first;
        if(tmp->p->pid== pid) {
            tmp->p->proc_state= state;
            return;
        }
    }
}
int send_sig_to_all(ProcList proc_list,int signal){
    struct node * tmp = proc_list->first;
    int num_proc_reciver = 0;
    for (; tmp!=NULL; tmp = tmp->next){
        if (tmp->p->proc_state == PROC_INFO_STATE_RUNNING){
            if (kill(tmp->p->pid, signal) >= 0 || errno == ESRCH)
                /**
                 * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
                 * the termination has not been read by main, in this case need wait on the proc to update the proc-list
                 * state
                 */
                ERROR_MESSAGE("PROCESS NOT FOUND MAY HAVE BEEN TERMINATED");

            else {
                if (errno == EINTR) { continue; }
                return -1;
                ERROR_MESSAGE("IMPOSSIBLE TO SEND TERMINATION SIGNAL TO KID");
            }
            num_proc_reciver++;
        }
    }
#ifdef DEBUG
    printf("SIG: %d HAS BEEN SENT TO: %d", signal, num_proc_reciver);
#endif
    return num_proc_reciver;
}
void proc_list_remove_head(ProcList self){
    if (proc_list_is_empty(self) == FALSE)
    {
        struct node *temp = self->first;
        if (self->first == self->last)
            self->first = self->last = NULL;
        else
            self->first = self->first->next;
        free(temp);
        self->num_proc--;
    } else
        proc_list_underflow();
}
void proc_list_underflow(){
    ERROR_MESSAGE("PROCESS LIST IS EMPTY");
}
