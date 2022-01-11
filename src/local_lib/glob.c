#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "headers/glob.h"
void int_to_hex(pid_t pid, char * buffer){
    sprintf(buffer, "0x%x", pid);
}
