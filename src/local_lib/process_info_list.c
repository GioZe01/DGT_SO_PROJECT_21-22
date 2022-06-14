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
    pid_t pid;
    int id_queue; /*Id for calculating range type of a user*/
    short int proc_state;
    short int proc_type;
    float budget;
    struct node *next;
};
struct processes_info_list {
    struct node *first;
    int num_proc;
};

/*  Helper Function Definitions */
/**
 * @brief Sort the list of processes by their budget
 * @param ref The reference to the first node of the list
 */
void sort_list(struct node **ref);

/**
 * @brief Split the list into two sublists
 * @param head  The head of the list to split
 * @param sublist1  The first sublist
 * @param sublist2  The second sublist
 */
void split_list(struct node *head, struct node **sublist1, struct node **sublist2);

/**
 * @brief Merge two sublists
 * @param sublist1  The first sublist
 * @param sublist2  The second sublist
 * @return The head of the merged list
 */
struct node *merge_list(struct node *sublist1, struct node *sublist2);

/**
 * @brief print the process
 * @param p The process to print
 */
void process_info_print(struct node *p);

/**
 * @brief remove the head of the list
 * @param self The list to remove the head from
 */
void proc_list_remove_head(ProcList self);

/**
 * @brief Advice via terminal that the list is in underflow
 */
void proc_list_underflow();

/* Function Implementations */
ProcList proc_list_create() {
    ProcList p = (struct processes_info_list *) malloc(sizeof(struct processes_info_list));
    if (p == NULL) {
        ERROR_MESSAGE("MALLOC FAILED IN THE CREATION OF QUEUE_T");
        return NULL;
    }
    p->first = NULL;
    p->num_proc = 0;
    return p;
}

Bool insert_in_list(ProcList *self, pid_t pid, int id_queue, short int proc_state, short int proc_type, float budget) {
    struct node *new = (struct node *) malloc(sizeof(struct node));
    if (new == NULL) {
        ERROR_MESSAGE("IMPOSSIBLE TO INSERT NEW PROC");
        return FALSE;
    }
    /** Copy the Proc pointer  into the new node */
    new->proc_state = proc_state;
    new->pid = pid;
    new->proc_type = proc_type;
    new->budget = budget;
    new->id_queue = id_queue;
    new->next = (*self)->first;
    (*self)->first = new;
    (*self)->num_proc++;
    return TRUE;
}

Bool proc_list_is_empty(ProcList self) {
    return (self->num_proc == 0) ? TRUE : FALSE;
}

struct ProcessInfo get_proc_from_queue_id(ProcList self, int id_queue) {
    /*Implemented linear search :( */
    struct node *tmp = self->first;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->id_queue == id_queue) {
            struct ProcessInfo info;
            info.budget = tmp->budget;
            info.pid = tmp->pid;
            info.proc_type = tmp->proc_type;
            info.proc_state = tmp->proc_state;
            return info;
        }
    };
}

struct ProcessInfo get_proc_from_pid(ProcList self, pid_t pid) {
    /*Implemented linear search :( */
    struct node *tmp = self->first;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->pid == pid) {
            struct ProcessInfo info;
            info.budget = tmp->budget;
            info.pid = tmp->pid;
            info.proc_type = tmp->proc_type;
            info.proc_state = tmp->proc_state;
            return info;
        }
    }
}

void print_list(ProcList self) {
    /* Sort the list before printing */
   sort_list(&(self->first));
    if (self->num_proc > MAX_PROC_TO_PRINT) {
        /* Print the min budget */
        struct node *tmp = self->first;
        int i;
        int num_proc = self->num_proc;
        for (i = 0; tmp != NULL; tmp = tmp->next) {
            if (i < MAX_PROC_TO_PRINT / 2 || i > num_proc - (MAX_PROC_TO_PRINT / 2)) {
                process_info_print(tmp);
                printf("\n");
            }
            i++;
        }
    } else {
        struct node *tmp = self->first;
        for (; tmp != NULL; tmp = tmp->next) {
            process_info_print(tmp);
            if (tmp->next != NULL)
                printf("\n");
        }
    }
    printf("\n");
}

void sort_list(struct node **ref) {
    struct node *head = *ref;
    struct node *sublist1;
    struct node *sublist2;

    if (head == NULL || head->next == NULL) {
        return;
    }
    split_list(head, &sublist1, &sublist2);

    sort_list(&sublist1);
    sort_list(&sublist2);
    *ref = merge_list(sublist1, sublist2);
}

void split_list(struct node *head, struct node **sublist1, struct node **sublist2) {
    struct node *slow = head;
    struct node *fast = head->next;
    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }
    *sublist1 = head;
    *sublist2 = slow->next;
    slow->next = NULL;
}

struct node *merge_list(struct node *sublist1, struct node *sublist2) {
    struct node *ris = NULL;
    if (sublist1 == NULL) {
        return sublist2;
    } else if (sublist2 == NULL) {
        return sublist1;
    }
    if (sublist1->budget < sublist2->budget) {
        ris = sublist1;
        ris->next = merge_list(sublist1->next, sublist2);
    } else {
        ris = sublist2;
        ris->next = merge_list(sublist1, sublist2->next);
    }
    return ris;
}

void process_info_print(struct node *p) {
    char state[80];
    from_procstate_to_string(p->proc_state, state);
    if (p != NULL)
        printf("# pid : %d | proc_type : %s | proc_state : %s | budget : %f ",
               p->pid,
               from_proctype_to_string(p->proc_type),
               state,
               p->budget);
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
        if (tmp->pid == pid) {
            tmp->proc_state = state;
            return;
        }
    }
}

int *send_sig_to_all(ProcList proc_list, int signal) {
    struct node *tmp = proc_list->first;
    int *pids = (int *) malloc(sizeof(int) * (proc_list->num_proc + 1));
    int num_proc_reciver = 0;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->proc_state == PROC_STATE_RUNNING && (kill(tmp->pid, signal) >= 0 || errno == ESRCH)) {
            /**
             * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
             * the termination has not been read by main, in this case need wait on the proc to update the proc-list
             * state
             */
#ifdef DEBUG_MAIN
            char string[80];
            from_procstate_to_string(tmp->proc_state, string);
            printf("\n{DEBUG_MAIN}:= SENT SIGNAL TO %d: process type: %s, proc exec_State: %s\n",
                   tmp->pid, from_proctype_to_string(tmp->proc_type), string);
#endif
            pids[num_proc_reciver + 1] = tmp->pid;
            num_proc_reciver++;
        } else if (errno == EINTR || tmp->proc_state == PROC_STATE_TERMINATED) {
            continue;
        } else {
            free(pids);
            return NULL;
        }
    }
    pids[0] = num_proc_reciver;

    return pids;
}

void proc_list_remove_head(ProcList self) {
    if (proc_list_is_empty(self) == FALSE) {
        struct node *temp = self->first;
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
        if (tmp->proc_state == PROC_STATE_RUNNING && (kill(tmp->pid, SIGINT) >= 0 || errno == ESRCH)) {
            /**
             * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
             * the termination has not been read by main, in this case need wait on the proc to update the proc-list
             * state
             */
            DEBUG_MESSAGE("PROC KILLED");
        } else {
            if (tmp->proc_state == PROC_STATE_TERMINATED || errno == EINTR) {
                continue;
            }
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
        check_msg_report(&msg_rep, queue_id, self);
        if (tmp->proc_state == PROC_STATE_RUNNING) {
            waitpid(tmp->pid, NULL, 0);
            tmp->proc_state = PROC_STATE_TERMINATED;
        }
    }
}

int get_num_of_proc(ProcList self) {
    return self->num_proc;
}

int get_num_of_user_proc_running(ProcList self) {
    if (self->first == NULL) {
        ERROR_MESSAGE("PROC LIST IS EMPTY");
        return 0;
    }
    struct node *tmp = self->first;
    int ris = 0;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->proc_type == PROC_TYPE_USER && tmp->proc_state == PROC_STATE_RUNNING) {
            ris++;
        }
    }
    return ris;
}

int get_num_of_node_proc_running(ProcList self) {
    if (self->first == NULL) {
        ERROR_MESSAGE("PROC LIST IS EMPTY");
        return 0;
    }
    struct node *tmp = self->first;
    int ris = 0;
    for (; tmp != NULL; tmp = tmp->next) {
        if (tmp->proc_type == PROC_TYPE_NODE && tmp->proc_state == PROC_STATE_RUNNING) {
            ris++;
        }
    }
    return ris;
}

int send_sig_to_all_nodes(ProcList proc_list, int signal, Bool exclude_last) {
    struct node *tmp = proc_list->first;
    int num_proc_reciver = 0;
    if (exclude_last == TRUE) {
        tmp = tmp->next;
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->proc_type == PROC_TYPE_NODE &&
                tmp->proc_state == PROC_STATE_RUNNING && (kill(tmp->pid, signal) >= 0 || errno == ESRCH)) {
                /**
                 * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
                 * the termination has not been read by main, in this case need wait on the proc to update the proc-list
                 * state
                 */
#ifdef DEBUG_MAIN
                char string[80];
                from_procstate_to_string(tmp->proc_state, string);
                printf("\n{DEBUG_MAIN}:= SENT SIGNAL TO [%d] : process type: %s, proc exec_State: %s\n",
                       tmp->pid, from_proctype_to_string(tmp->proc_type), string);
                DEBUG_MESSAGE("SIGNAL SENT TO THE PROCESS");
#endif
                num_proc_reciver++;
            } else if (errno == EINTR || tmp->proc_state == PROC_STATE_TERMINATED) {
                continue;
            } else {
                return -1;
            }
        }
    } else {
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->proc_type == PROC_TYPE_NODE && tmp->proc_state == PROC_STATE_RUNNING &&
                (kill(tmp->pid, signal) >= 0 || errno == ESRCH)) {
                /**
                 * errno == ESRCH is allowed because it might be that the proc intrest is terminated and
                 * the termination has not been read by main, in this case need wait on the proc to update the proc-list
                 * state
                 */
#ifdef DEBUG_MAIN
                char string[80];
                from_procstate_to_string(tmp->proc_state, string);
                printf("\n{DEBUG_MAIN}:= SENT SIGNAL TO : process type: %s, proc exec_State: %s\n",
                       from_proctype_to_string(tmp->proc_type), string);
                DEBUG_MESSAGE("SIGNAL SENT TO THE PROCESS");
#endif
                num_proc_reciver++;
            } else if (errno == EINTR || tmp->proc_state == PROC_STATE_TERMINATED) {
                continue;
            } else {
                return -1;
            }
        }
    }
    return num_proc_reciver;
}

Bool send_msg_to_all_nodes(int queue_id, int retry, ProcList proc_list, int node_id, Bool exclude_last) {
    struct node *tmp = proc_list->first;
    if (exclude_last == TRUE) {
        tmp = tmp->next;
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->proc_type == PROC_TYPE_NODE &&
                tmp->proc_state == PROC_STATE_RUNNING) {
                struct node_msg msg;
                struct Transaction t = create_empty_transaction();
                t.sender = node_id;
                node_msg_snd(queue_id, &msg, MSG_MASTER_ORIGIN_ID, &t, getpid(), TRUE, retry, tmp->id_queue);
            }
        }
    } else {
        for (; tmp != NULL; tmp = tmp->next) {
            if (tmp->proc_type == PROC_TYPE_NODE && tmp->proc_state == PROC_STATE_RUNNING) {
                struct node_msg msg;
                struct Transaction *t = (struct Transaction *) malloc(sizeof(struct Transaction));
                t->sender = node_id;
                node_msg_snd(queue_id, &msg, MSG_MASTER_ORIGIN_ID, t, getpid(), TRUE, retry, tmp->id_queue);
            }
        }
    }
    return TRUE;
}


struct ProcessInfo get_first(ProcList self) {
    struct ProcessInfo info;
    info.budget = self->first->budget;
    info.pid = self->first->pid;
    info.proc_type = self->first->proc_type;
    info.proc_state = self->first->proc_state;
    return info;
}

int update_proc(ProcList self, int pid, float budget, short int proc_state) {
    if (budget < 0) {
        return -1;
    }
    struct node *tmp = self->first;
    while (tmp != NULL) {
        if (tmp->pid == pid) {
            tmp->budget = budget;
            tmp->proc_state = proc_state;
            return 0;
        }
        tmp = tmp->next;
    }
    return -2;
}

void get_random_node_list(ProcList proc_list, ProcList node_list, int num_of_node) {
    int running_node = get_num_of_node_proc_running(proc_list);
    if (num_of_node > running_node) {
        ERROR_MESSAGE("NOT ENOUGH NODE PROCESSES RUNNING");
        node_list = proc_list;
        return;
    }
    int random_node, last_node = -1;
    int i = 0;
    ProcList running_nodes = get_running_node_proc(proc_list);

    printf("NODE LIST\n\n");
    print_list(running_nodes);
    for (; i < num_of_node; i++) {
        while (random_node == last_node) {
            random_node = rand() % running_node + 1;
        }
        struct node *tmp = running_nodes->first;
        int j = 0;
        for (; j < random_node; j++) {
            tmp = tmp->next;
        }
        insert_in_list(node_list, tmp->pid, tmp->id_queue, tmp->proc_state, tmp->proc_type, tmp->budget);
    }
}

ProcList get_running_node_proc(ProcList self) {
    ProcList node_list = proc_list_create();
    struct node *tmp = self->first;
    while (tmp != NULL) {
        if (tmp->proc_type == PROC_TYPE_NODE && tmp->proc_state == PROC_STATE_RUNNING) {
            insert_in_list(node_list, tmp->pid, tmp->id_queue, tmp->proc_state, tmp->proc_type, tmp->budget);
        }
        tmp = tmp->next;
    }
    return node_list;
}