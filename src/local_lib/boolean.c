#include "headers/boolean.h"
char *bool_to_string(Bool b){
    if(b == TRUE){
        return "true";
    }
    else{
        return "false";
    }
}