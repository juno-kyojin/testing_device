CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS = -lpthread

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
EXECUTABLE = $(BIN_DIR)/testing_device

all: directories $(EXECUTABLE)

directories:
    mkdir -p $(OBJ_DIR) $(BIN_DIR) logs

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
    $(CC) $(CFLAGS) -c $< -o $@

$(EXECUTABLE): $(OBJECTS)
    $(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
    rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean directories