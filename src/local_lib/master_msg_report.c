#define _GNU_SOURCE
/* Std */
#include <stdio.h>
#include <errno.h>
/* Sys */
#include <sys/msg.h>
/* Local */
#include "headers/master_msg_report.h"
#include "headers/glob.h"
#include "headers/process_info_list.h"
#include "headers/simulation_errors.h"
#include "headers/debug_utility.h"


void master_msg_report_create(struct master_msg_report *self, long type,long proc_type, pid_t sender_pid, short int state, float budget) {
    self->type = type;
    self->proc_type = proc_type;
    self->sender_pid = sender_pid;
    self->state = state;
    if (type == INFO_BUDGET){
        self->budget = budget;
    }
    self->budget = 0;
}

void master_msg_report_print(const struct master_msg_report *self) {
    char state_string [80];
    switch (self->type) {
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
        case IMPOSSIBLE_TO_CONNECT_TO_SHM:
            switch (self->state) {
                case PROC_STATE_WAITING:
                case PROC_STATE_NODE_SERV_TRANS:
                    printf("[MASTER_MSG_REPORT]  := msg_type: %s%s%s%s | porc_type: %s | sender: %d | state: %s%s%d%s \n", COLOR_RESET_ANSI_CODE,
                           COLOR_RED_ANSI_CODE, from_type_to_string(self->type),COLOR_RESET_ANSI_CODE,from_proctype_to_string(self->proc_type),self-> sender_pid,COLOR_RESET_ANSI_CODE, COLOR_YELLOW_ANSI_CODE ,self->state, COLOR_RESET_ANSI_CODE);
                    break;
                default:
                    printf("[MASTER_MSG_REPORT]  := msg_type: %s%s%s%s | porc_type: %s  | sender: %d | state: %d \n", COLOR_RESET_ANSI_CODE,
                           COLOR_RED_ANSI_CODE, from_type_to_string(self->type), COLOR_RESET_ANSI_CODE,from_proctype_to_string(self->proc_type),self->sender_pid, self->state);
                    break;
            }
            break;
        default:
            from_procstate_to_string(self->state, state_string);
            printf("[MASTER_MSG_REPORT]  := msg_type: %s | proc_type: %s | sender: %d | state: %s \n", from_type_to_string(self->type),from_proctype_to_string(self->proc_type) ,self->sender_pid,state_string);
    }
}
void from_procstate_to_string(int state,char * string ){
    strcpy(string, COLOR_RESET_ANSI_CODE);
    switch(state){
        case PROC_STATE_WAITING:
            strcat(string , COLOR_YELLOW_ANSI_CODE);
            strcat(string, "WAITING");
            break;
        case PROC_STATE_RUNNING:
            strcat(string, COLOR_GREEN_ANSI_CODE);
            strcat(string, "RUNNING");
            break;
        case PROC_STATE_TERMINATED:
            strcat(string, COLOR_RED_ANSI_CODE);
            strcat(string, "ENDED");
            break;
        case PROC_STATE_INIT:
            strcat(string,"INIT");
            break;
        default:
            strcat(string,"ERROR");
            break;
    }
    strcat(string, COLOR_RESET_ANSI_CODE);
}

char * from_proctype_to_string(long proc_type){
    switch(proc_type){
        case USER:
            return "USER";
        case NODE:
            return "NODE";
        case NODE_TP:
            return "NODE_TP";
        default:
            return "UNDEFINED PROC TYPE";
    }
}
char * from_type_to_string(long type){
    switch(type){
        case TERMINATION_END_CORRECTLY:
            return "TERMINATION END CORRECTLY";
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
            return "IMPOSSIBLE TO SEND TRANSACTION";
        case IMPOSSIBLE_TO_CONNECT_TO_SHM:
            return "IMPOSSIBLE TO CONNECT TO SHM";
        case SIGNALS_OF_TERM_RECEIVED:
            return "SIGNALS OF TERM RECEIVED";
        case INFO_BUDGET:
            return "INFO BUDGET";
        case UNUSED_PROC:
            return "UNUSED PROCESS";
        default:
            return "";
    }
}
int master_msg_send(int id, struct master_msg_report * self,long type,long proc_type, pid_t sender_pid, short int state, Bool create, float budget){
    if (create == TRUE){master_msg_report_create(self,type,proc_type,sender_pid, state, budget);}
#ifdef DEBUG_USER
    char string [80];
    from_procstate_to_string(self->state, string);
    printf("\nqueue id: %d | process_type: %s | process_state: %s \n",id,from_proctype_to_string(self->proc_type), string);
#endif
    while(msgsnd(id, self, sizeof(struct master_msg_report)-sizeof(long),0)<0){
        if (errno != EINTR)return -1;
    }
    return 0;
}
int master_msg_receive(int id, struct master_msg_report * self){
    if(msgrcv(id, self, sizeof(struct master_msg_report)-sizeof(long),0/*all incoming messages*/,IPC_NOWAIT)<0){
        if(errno == ENOMSG){
            return -2;
        }
        return -1;
    }
    return 0;
}
int master_msg_receive_info(int id, struct master_msg_report * self){
    if(msgrcv(id, self, sizeof(struct master_msg_report)-sizeof(long),INFO_BUDGET,0)<0){
        if(errno == ENOMSG){
            return -2;
        }
        return -1;
    }
    return 0;
}

int acknowledge(struct master_msg_report * self, ProcList list){
    long msg_type = self->type;
    int proc_type = self->proc_type;
    short int exec_state = self->state;
    Proc proc_to_update = get_proc_from_pid(list,self->sender_pid);
    if (proc_to_update == NULL){
        return -2;
    }
    switch(msg_type){
        case TERMINATION_END_CORRECTLY:
        case IMPOSSIBLE_TO_SEND_TRANSACTION:
        case SIGNALS_OF_TERM_RECEIVED:
        case UNUSED_PROC:
        case INFO_BUDGET:
            if (self->budget>=0){
                proc_to_update->budget = self->budget;
            }
            proc_to_update->proc_state= exec_state;
            break;
        default:
            return -1;
    }
    return 0;
}
int check_msg_report(struct master_msg_report *msg_report, int msg_report_id_master, ProcList proc_list) {
    struct msqid_ds msg_rep_info;
    if (msgctl(msg_report_id_master, IPC_STAT, &msg_rep_info) < 0) {
        DEBUG_ERROR_MESSAGE("IMPOSSIBLE TO RETRIEVE MESSAGE QUEUE INFO");
        return -1;
    } else {
        /*fetching all msg if present*/
        if (msg_rep_info.msg_qnum != 0 &&
            msgrcv(msg_report_id_master, msg_report, sizeof(*msg_report) - sizeof(long), 0, 0)>0) {
            if (acknowledge(msg_report, proc_list)<0){
                ERROR_MESSAGE("IMPOSSIBLE TO MAKE THE ACKNOWLEDGE OF MASTER MESSAGE");
            }
        }
        return 0;
    }
}
