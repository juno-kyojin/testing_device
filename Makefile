# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -Iinclude -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lcjson

# Directories
SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build
INCLUDE_DIR = include
ACTION_DIR = $(SRC_DIR)/action
CONFIG_DIR = $(SRC_DIR)/config
CORE_DIR = $(SRC_DIR)/core
PARSER_DIR = $(SRC_DIR)/parser
TEST_DIR = $(SRC_DIR)/test
UTILS_DIR = $(SRC_DIR)/utils

# Output executable
TARGET = $(BUILD_DIR)/testing_device

# Source files
SOURCES = \
    $(ACTION_DIR)/action.c \
    $(ACTION_DIR)/action_registry.c \
    $(CONFIG_DIR)/app_config.c \
    $(CORE_DIR)/log.c \
    $(CORE_DIR)/main.c \
    $(PARSER_DIR)/parser.c \
    $(PARSER_DIR)/parser_dynamic.c \
    $(PARSER_DIR)/parser_hardcode.c \
    $(TEST_DIR)/ping.c \
    $(TEST_DIR)/test_executor.c \
    $(UTILS_DIR)/file_process.c

# Object files
OBJECTS = \
    $(BIN_DIR)/action.o \
    $(BIN_DIR)/action_registry.o \
    $(BIN_DIR)/app_config.o \
    $(BIN_DIR)/log.o \
    $(BIN_DIR)/main.o \
    $(BIN_DIR)/parser.o \
    $(BIN_DIR)/parser_dynamic.o \
    $(BIN_DIR)/parser_hardcode.o \
    $(BIN_DIR)/ping.o \
    $(BIN_DIR)/test_executor.o \
    $(BIN_DIR)/file_process.o

# Header files
HEADERS = \
    $(INCLUDE_DIR)/action/action.h \
    $(INCLUDE_DIR)/action/action_registry.h \
    $(INCLUDE_DIR)/config/app_config.h \
    $(INCLUDE_DIR)/core/log.h \
    $(INCLUDE_DIR)/core/types.h \
    $(INCLUDE_DIR)/parser/parser.h \
    $(INCLUDE_DIR)/parser/parser_dynamic.h \
    $(INCLUDE_DIR)/parser/parser_hardcode.h \
    $(INCLUDE_DIR)/test/ping.h \
    $(INCLUDE_DIR)/test/test_executor.h \
    $(INCLUDE_DIR)/utils/file_process.h

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files to object files
$(BIN_DIR)/%.o: $(ACTION_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(CONFIG_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(CORE_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(PARSER_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(TEST_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(UTILS_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

# Phony targets
.PHONY: all clean