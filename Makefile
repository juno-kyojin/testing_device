CC = gcc
CFLAGS = -I./include -Wall -Wextra -g -fPIC
LDFLAGS = -pthread -lcjson -ldl

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
PLUGIN_DIR = plugins

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXECUTABLE = $(BIN_DIR)/testing_device

# Đảm bảo rằng các symbols được xuất cho plugins
CFLAGS += -rdynamic

all: directories $(EXECUTABLE)

directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(PLUGIN_DIR)
	mkdir -p var/log
	mkdir -p var/input
	mkdir -p var/results

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
plugins: directories
	chmod +x scripts/compile_plugin.sh
	./scripts/compile_plugin.sh plugins/ping_plugin.c
	./scripts/compile_plugin.sh plugins/speedtest_plugin.c

clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)
	
clean_plugins:
	rm -f $(PLUGIN_DIR)/*.so

clean_all: clean clean_plugins
	rm -f var/log/* var/results/*
	rm -rf var/input/processed

.PHONY: all clean clean_plugins clean_all directories plugins