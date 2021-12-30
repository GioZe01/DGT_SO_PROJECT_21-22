#TODO: mkdir maker
#TODO: DEF a tempo di compilazione
#Flags per la compilazione
CFLAGS = -std=c89 -pedantic -Wall -Wextra

LIBRARIES = -lm
MKDIR_P = mkdir -p

# Folders
BUILDING_DIR = build
BUILD_OBJECT_DIR = $(BUILDING_DIR)/obj
BUILD_BIN_DIR = $(BUILDING_DIR)/bin
SOURCE_DIR = src
SOURCE_HEADERS_DIR = $(SOURCE_DIR)/local_lib/headers
SOURCE_IMPL_DIR =  $(SOURCE_DIR)/local_lib

#Create Build Folder


# Dipendenze
_DEPENDENCIES_MAIN = conf_file.h glob_vars.h boolean.h simulation_errors.h
DEPENDENCIES_MAIN = $(patsubst %, $(SOURCE_HEADERS_DIR)/%, $(_DEPENDENCIES_MAIN))
# Objects
_OBJECTS_MAIN = conf_file.o main.o simulation_errors.o
OBJECTS_MAIN = $(patsubst %, $(BUILD_OBJECT_DIR)/%, $(_OBJECTS_MAIN))


$(BUILD_OBJECT_DIR)/%.o: $(SOURCE_IMPL_DIR)/%.c Makefile
	$(CC) -c -o $@ $< $(CFLAGS)
$(BUILD_OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c Makefile
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD_BIN_DIR)/main: $(OBJECTS_MAIN)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBRARIES)

#TODO: Aggiungere altri anche qui
all: $(BUILD_BIN_DIR)/main

run: all
	./$(BUILD_BIN_DIR)/main
clean:
	$(RM) $(BUILD_OBJECT_DIR)/* $(BUILD_BIN_DIR)/*

prova:
	echo $(OBJECTS_MAIN)