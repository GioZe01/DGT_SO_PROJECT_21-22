#TODO: mkdir maker
#Flags per la compilazione
CC= gcc
CFLAGS = -std=c89 -pedantic -Wall -Wextra -Werror=format-security -fstack-protector-strong

# Folders
BUILDING_DIR = build
BUILD_OBJECT_DIR = $(BUILDING_DIR)/obj
BUILD_BIN_DIR = $(BUILDING_DIR)/bin
SOURCE_DIR = src
SOURCE_HEADERS_DIR = $(SOURCE_DIR)/local_lib/headers
SOURCE_IMPL_DIR =  $(SOURCE_DIR)/local_lib

# Utilities
LIBRARIES = -lm
MKDIR_P = mkdir -p
#Create Build Folder


#DEBUG DEBUG_NODE
# DEFINITION AT COMP TIME
_DEF_COMP_TIME = SO_BLOCK_SIZE=100 SO_REGISTRY_SIZE=1000 _GNU_SOURCE DEBUG DEBUG_MAIN DEBUG_NODE #DEBUG_USER
DEF_COMP_TIME = $(patsubst %, -D%, $(_DEF_COMP_TIME))



# Dependencies
_DEPENDENCIES_MAIN =  glob.h conf_file.h boolean.h simulation_errors.h debug_utility.h process_info_list.h semaphore.h user_msg_report.h master_msg_report.h node_msg_report.h shm_conf.h book_master_shm.h int_condenser.h transaction_list.h gt_sig_handler.h
DEPENDENCIES_MAIN = $(patsubst %, $(SOURCE_HEADERS_DIR)/%, $(_DEPENDENCIES_MAIN))
_DEPENDENCIES_USER = conf_file.h boolean.h simulation_errors.h debug_utility.h semaphore.h process_info_list.h transaction_list.h user_transaction.h user_msg_report.h glob.h master_msg_report.h node_msg_report.h shm_conf.h book_master_shm.h int_condenser.h gt_sig_handler.h
DEPENDENCIES_USER = $(patsubst %, $(SOURCE_HEADERS_DIR)/%, $(_DEPENDENCIES_USER))
_DEPENDENCIES_NODE = conf_file.h boolean.h simulation_errors.h debug_utility.h semaphore.h transaction_list.h process_info_list.h node_transactor.h node_msg_report.h glob.h master_msg_report.h node_msg_report.h shm_conf.h book_master_shm.h node_tp_shm.h int_condenser.h gt_sig_handler.h
DEPENDENCIES_NODE = $(patsubst %, $(SOURCE_HEADERS_DIR)/%, $(_DEPENDENCIES_NODE))
# Objects
_OBJECTS_MAIN = main.o conf_file.o process_info_list.o semaphore_wrap.o user_msg_report.o master_msg_report.o node_msg_report.o shm_conf.o book_master_shm.o int_condenser.o transaction_list.o gt_sig_handler.o boolean.o
OBJECTS_MAIN = $(patsubst %, $(BUILD_OBJECT_DIR)/%, $(_OBJECTS_MAIN))
_OBJECTS_USER = user_proc.o process_info_list.o conf_file.o semaphore_wrap.o user_transaction.o transaction_list.o user_msg_report.o node_msg_report.o master_msg_report.o shm_conf.o int_condenser.o gt_sig_handler.o boolean.o
OBJECTS_USER = $(patsubst %, $(BUILD_OBJECT_DIR)/%, $(_OBJECTS_USER))
_OBJECTS_NODE = node_proc.o process_info_list.o conf_file.o semaphore_wrap.o node_transactor.o transaction_list.o user_msg_report.o node_msg_report.o master_msg_report.o shm_conf.o book_master_shm.o int_condenser.o int_condenser.o gt_sig_handler.o boolean.o
OBJECTS_NODE = $(patsubst %, $(BUILD_OBJECT_DIR)/%, $(_OBJECTS_NODE))

$(BUILD_OBJECT_DIR)/%.o: $(SOURCE_IMPL_DIR)/%.c Makefile
	$(CC) -c -o $@ $< $(CFLAGS) $(DEF_COMP_TIME)
$(BUILD_OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c Makefile
	$(CC) -c -o $@ $< $(CFLAGS) $(DEF_COMP_TIME)

$(BUILD_BIN_DIR)/main: $(OBJECTS_MAIN)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBRARIES)

$(BUILD_BIN_DIR)/user: $(OBJECTS_USER)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBRARIES)

$(BUILD_BIN_DIR)/node: $(OBJECTS_NODE)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBRARIES)

all: $(BUILD_BIN_DIR)/main $(BUILD_BIN_DIR)/user $(BUILD_BIN_DIR)/node

run: all
	./$(BUILD_BIN_DIR)/main
clean:
	$(RM) $(BUILD_OBJECT_DIR)/* $(BUILD_BIN_DIR)/*
