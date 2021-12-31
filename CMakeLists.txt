cmake_minimum_required(VERSION 3.16)
project(DGT_SO_PROJECT_21_22 C)

set(CMAKE_C_STANDARD 11)

include_directories(src/local_lib/headers)

add_executable(DGT_SO_PROJECT_21_22
        src/local_lib/headers/conf_file.h
        src/local_lib/headers/glob_vars.h
        src/local_lib/conf_file.c
        src/main.c)
