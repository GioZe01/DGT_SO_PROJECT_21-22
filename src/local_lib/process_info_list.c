/* Std */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
/* Sys */
#include <sys/types.h>
#include <sys/wait.h>
/* Local */
#include "headers/glob.h"
#include "headers/process_info_list.h"
#include "headers/simulation_errors.h"
#include "headers/master_msg_report.h" /*imported for proc state enum type*/
#include "headers/node_msg_report.h"

#ifdef DEBUG

#include "headers/debug_utility.h"

#else /*unimplemented*/
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex)
#define DEBUG_MESSAGE(mex)
#define DEBUG_SIGNAL(mex, signum)
#define DEBUG_ERROR_MESSAGE(mex)
#endif

/* Struct definitions */
struct node {
    Proc p;
    struct node *next;
};
struct processes_info_list {
    struct node *first;
    struct node *last;
    int num_proc;
};

/*  Helper Funciton */

/**
 * @brief Split head into two sublists
 * @param head The head of the list
 * @param frontRef A pointer to the head of the first sublist
 * @param backRef A pointer to the head of the second sublist
 */
void frontBackSplit(struct node *head, struct node **frontRef, struct node **backRef);

/**
 * @brief Merge Sort the list
 * @param head The head of the list
 */
void mergeSort(struct node **head);

/**
 * @brief Merge two sublists
 * @param frontHead The head of the first sublist
 * @param backHead The head of the second sublist
 * @return The head of the merged sublist
 */
struct node *sortedMerge(struct node *frontHead, struct node *backHead);

/**
 * @brief print the process
 * @param p The process to print
 */
void process_info_print(const Proc p);

/**
 * @brief remove the head of the list
 * @param self The list to remove the head from
 */
void proc_list_remove_head(ProcList self);

void proc_list_underflow();

ProcList proc_list_create() {
    ProcList p = malloc(sizeof(struct processes_info_list));
    if (p== NULL) {
        ERROR_MESSAGE("MALLOC FAILED IN THE CREATION OF QUEUE_T");
        return NULL;
    }
    p->first = NULL;
    p->last = NULL;
    p->num_proc = 0;
    return p;
}

void insert_in_list(ProcList self, pid_t pid, short int type, int queue_id) {
    struct node *new = (struct node *)malloc(sizeof(struct node));
    if (new == NULL) {
        ERROR_MESSAGE("IMPOSSIBLE TO INSERT NEW PROC");
        return;
    }
    new->p = (Proc)malloc(sizeof(Proc));
    if (new->p == NULL) {
        ERROR_MESSAGE("IMPOSSIBLE TO INSERT NEW PROC");
        return;
    } else {
        new->next = NULL;
        new->p->proc_type = type;
        new->p->proc_state = PROC_STATE_RUNNING;
        new->p->id_queue = queue_id;
        new->p->budget = -1;
        new->p->pid = pid;
        if (proc_list_is_empty(self) == TRUE) {
            self->first = self->last = new;
        } else {
            self->last->next = new;
            self->last = new;
        }
        self->num_proc++;
    }
}

Bool proc_list_is_empty(const ProcList self) {
    return (self->num_proc == 0) ? TRUE : FALSE;
}

Proc get_proc_from_queue_id(ProcList self, int id_queue) {
    /*Implemented linear search :( */
    struct node *tmp = self->first;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->p->id_queue == id_queue) return tmp->p;
    }
    return NULL;
}

Proc get_proc_from_pid(ProcList self, pid_t pid) {
    /*Implemented linear search :( */
    struct node *tmp = self->first;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->p->pid == pid) return tmp->p;
    }
    return NULL;
}


void print_list(ProcList self) {
    mergeSort(&self->first);
    if (self->num_proc > MAX_PROC_TO_PRINT){
        /* Print the min budget */
        struct node *tmp = self->first;
        int i;
        int num_proc = self->num_proc;
        for (i=0; tmp != NULL; tmp = tmp->next) {
            if (i< MAX_PROC_TO_PRINT/2) {
                process_info_print(tmp->p);
                printf("\n");
            }else if (i>= num_proc - MAX_PROC_TO_PRINT/2){
                process_info_print(tmp->p);
                printf("\n");
            }
            i++;
        }

    }else{
        struct node *tmp = self->first;
        for (; tmp != NULL; tmp = tmp->next) {
            process_info_print(tmp->p);
            if (tmp->next != NULL) printf("\n");
        }
    }
    printf("\n");
}


void process_info_print(const Proc p) {
    char state [80];
    from_procstate_to_string(p->proc_state, state);
    if (p != NULL)
        printf("# pid : %d | proc_type : %s | proc_state : %s | budget : %f ",
               p->pid,
               from_proctype_to_string(p->proc_type),
               state,
               p->budget
        );
    else
            DEBUG_ERROR_MESSAGE("Calling print_list on NULL process info");
}

void list_free(ProcList self) {
    DEBUG_NOTIFY_ACTIVITY_RUNNING("PROCESSES INFO LIST FREE...");
    while (proc_list_is_empty(self) == FALSE) {
        proc_list_remove_head(self);
    }
    DEBUG_NOTIFY_ACTIVITY_DONE("PROCESSES INFO LIST FREE DONE");
}

void list_set_state(ProcList self, pid_t pid, short int state) {
    /*linear search implemented*/
    struct node *tmp = self->first;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->p->pid == pid) {
            tmp->p->proc_state = state;
            return;
        }
    }
}

int send_sig_to_all(ProcList proc_list, int signal) {
    struct node *tmp = proc_list->first;
    int num_proc_reciver = 0;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->p->proc_state == PROC_STATE_RUNNING && (kill(tmp->p->pid, signal) >= 0 || errno == ESRCH)){
            /**
             * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
             * the termination has not been read by main, in this case need wait on the proc to update the proc-list
             * state
             */
#ifdef DEBUG_MAIN
        char string [80];
        from_procstate_to_string(tmp->p->proc_state,string);
        printf ("\n{DEBUG_MAIN}:= SENT SIGNAL TO : process type: %s, proc exec_State: %s\n", from_proctype_to_string(tmp->p->proc_type),string);
            DEBUG_MESSAGE("SIGNAL SENT TO THE PROCESS");
#endif
         num_proc_reciver++;
        } else if (errno == EINTR || tmp->p->proc_state == PROC_STATE_TERMINATED) { continue; }
        else {
            return -1;
        }
    }

    return num_proc_reciver;
}

void proc_list_remove_head(ProcList self) {
    if (proc_list_is_empty(self) == FALSE) {
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

void proc_list_underflow() {
    ERROR_MESSAGE("PROCESS LIST IS EMPTY");
}

void terminator(ProcList self) {
    struct node *tmp = self->first;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->p->proc_state == PROC_STATE_RUNNING && (kill(tmp->p->pid, SIGINT) >= 0 || errno == ESRCH)) {
            /**
             * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
             * the termination has not been read by main, in this case need wait on the proc to update the proc-list
             * state
             */
            DEBUG_MESSAGE("PROC KILLED");
        } else {
            if (tmp->p->proc_state == PROC_STATE_TERMINATED|| errno == EINTR) { continue; }
            ERROR_MESSAGE("IMPOSSIBLE TO SEND TERMINATION SIGNAL TO KID");
        }
    }
}

void saving_private_ryan(ProcList self, int queue_id) {
    if (self->first == NULL) {
        return;
    }
    struct node *tmp = self->first;
    struct master_msg_report msg_rep;
    for (; tmp != NULL; tmp = tmp->next) {
        check_msg_report(&msg_rep,queue_id,self);
        if (tmp->p->proc_state == PROC_STATE_RUNNING) {
            waitpid(tmp->p->pid, NULL, 0);
            tmp->p->proc_state = PROC_STATE_TERMINATED;
        }
    }
}
int get_num_of_proc(ProcList self) {
    return self->num_proc;
}
int get_num_of_user_proc_running(ProcList self){
    if (self->first == NULL){
        return 0;
    }
    struct node * tmp = self->first;
    int ris = 0;
    for(;tmp != NULL; tmp = tmp->next){
        if (tmp->p->proc_type == PROC_TYPE_USER && tmp->p->proc_state == PROC_STATE_RUNNING){
            ris++;
        }
    }
    return ris;
}

int send_sig_to_all_nodes(ProcList proc_list,int signal, Bool exclude_last){
    struct node *tmp = proc_list->first;
    struct node *last = proc_list->last;
    int num_proc_reciver = 0;
    if (exclude_last == TRUE){
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->p->proc_type == PROC_TYPE_NODE && tmp->p->pid != last->p->pid && tmp->p->proc_state == PROC_STATE_RUNNING && (kill(tmp->p->pid, signal) >= 0 || errno == ESRCH)){
                /**
                 * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
                 * the termination has not been read by main, in this case need wait on the proc to update the proc-list
                 * state
                 */
#ifdef DEBUG_MAIN
                char string [80];
                from_procstate_to_string(tmp->p->proc_state,string);
                printf ("\n{DEBUG_MAIN}:= SENT SIGNAL TO : process type: %s, proc exec_State: %s\n", from_proctype_to_string(tmp->p->proc_type),string);
                DEBUG_MESSAGE("SIGNAL SENT TO THE PROCESS");
#endif
                num_proc_reciver++;
            } else if (errno == EINTR || tmp->p->proc_state == PROC_STATE_TERMINATED) { continue; }
            else {
                return -1;
            }
        }
    }
    else{
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->p->proc_type == PROC_TYPE_NODE && tmp->p->proc_state == PROC_STATE_RUNNING && (kill(tmp->p->pid, signal) >= 0 || errno == ESRCH)){
                /**
                 * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
                 * the termination has not been read by main, in this case need wait on the proc to update the proc-list
                 * state
                 */
#ifdef DEBUG_MAIN
                char string [80];
                from_procstate_to_string(tmp->p->proc_state,string);
                printf ("\n{DEBUG_MAIN}:= SENT SIGNAL TO : process type: %s, proc exec_State: %s\n", from_proctype_to_string(tmp->p->proc_type),string);
                DEBUG_MESSAGE("SIGNAL SENT TO THE PROCESS");
#endif
                num_proc_reciver++;
            } else if (errno == EINTR || tmp->p->proc_state == PROC_STATE_TERMINATED) { continue; }
            else {
                return -1;
            }
        }
    }
    return num_proc_reciver;
}

Bool send_msg_to_all_nodes(int queue_id, int retry, ProcList proc_list, int node_id, Bool exclude_last){
    struct node *tmp = proc_list->first;
    struct node *last = proc_list->last;
    int num_proc_reciver = 0;
    if (exclude_last == TRUE){
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->p->proc_type == PROC_TYPE_NODE && tmp->p->pid != last->p->pid && tmp->p->proc_state == PROC_STATE_RUNNING){
                struct node_msg msg;
                struct Transaction*t = (struct Transaction*)malloc(sizeof(struct Transaction));
                t->sender = node_id;
                node_msg_snd(queue_id,&msg, MSG_MASTER_ORIGIN_ID, t, getpid(), TRUE, retry, tmp->p->id_queue);
            }
        }
    }else{
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->p->proc_type == PROC_TYPE_NODE && tmp->p->proc_state == PROC_STATE_RUNNING){
                struct node_msg msg;
                struct Transaction*t = (struct Transaction*)malloc(sizeof(struct Transaction));
                t->sender = node_id;
                node_msg_snd(queue_id,&msg, MSG_MASTER_ORIGIN_ID, t, getpid(), TRUE, retry, tmp->p->id_queue);
            }
        }
    }
    return TRUE;
}

void mergeSort(struct node **head_ref){
    struct node * head = *head_ref;
    struct node * sublist1;
    struct node * sublist2;
    if ((head == NULL) || (head->next == NULL)){
        return;
    }
    frontBackSplit(head, &sublist1, &sublist2);
    mergeSort(&sublist1);
    mergeSort(&sublist2);
    *head_ref = sortedMerge(sublist1, sublist2);
}


void frontBackSplit(struct node *head, struct node **frontRef, struct node **backRef) {
    struct node *fast;
    struct node *slow;
    if (head == NULL || head->next == NULL) {
        *frontRef = head;
        *backRef = NULL;
    } else {
        slow = head;
        fast = head->next;
        while (fast != NULL) {
            fast = fast->next;
            if (fast != NULL) {
                slow = slow->next;
                fast = fast->next;
            }
        }
        *frontRef = head;
        *backRef = slow->next;
        slow->next = NULL;
    }
}


struct node *sortedMerge(struct node *frontHead, struct node *backHead){
    struct node *result = NULL;
    if (frontHead == NULL) {
        return backHead;
    }
    else if (backHead == NULL) {
        return frontHead;
    }
    if (frontHead->p->budget < backHead->p->budget) {
        result = frontHead;
        result->next = sortedMerge(frontHead->next, backHead);
    }
    else {
        result = backHead;
        result->next = sortedMerge(frontHead, backHead->next);
    }
    return result;
}
