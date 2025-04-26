# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -Iinclude -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lcjson -lm  # Added -lm to link the math library

# Directories
SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build
INCLUDE_DIR = include
CORE_DIR = $(SRC_DIR)/core
TEST_DIR = $(SRC_DIR)/test

# Output executable
TARGET = $(BUILD_DIR)/testing_device

# Source files
SOURCES = \
    $(CORE_DIR)/action.c \
    $(CORE_DIR)/action_registry.c \
    $(CORE_DIR)/file_process.c \
    $(CORE_DIR)/log.c \
    $(CORE_DIR)/main.c \
    $(CORE_DIR)/parser.c \
    $(TEST_DIR)/speedtest.c \
    $(TEST_DIR)/ping.c

# Object files
OBJECTS = \
    $(BIN_DIR)/action.o \
    $(BIN_DIR)/action_registry.o \
    $(BIN_DIR)/file_process.o \
    $(BIN_DIR)/log.o \
    $(BIN_DIR)/main.o \
    $(BIN_DIR)/parser.o \
    $(BIN_DIR)/speedtest.o \
    $(BIN_DIR)/ping.o

# Header files
HEADERS = \
    $(INCLUDE_DIR)/action.h \
    $(INCLUDE_DIR)/action_registry.h \
    $(INCLUDE_DIR)/file_process.h \
    $(INCLUDE_DIR)/log.h \
    $(INCLUDE_DIR)/parser.h \
    $(INCLUDE_DIR)/speedtest.h \
    $(INCLUDE_DIR)/ping.h \
    $(INCLUDE_DIR)/types.h

# Default target
all: $(TARGET) deploy restart_service

# Link object files to create executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files to object files
$(BIN_DIR)/%.o: $(CORE_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(TEST_DIR)/%.c $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Stop the service before deploying
stop_service:
	sudo systemctl stop testing_device.service || true
	@echo "Stopped testing_device.service"

# Sao chép file thực thi đến /usr/local/bin/
deploy: stop_service
	sudo cp $(TARGET) /usr/local/bin/testing_device
	sudo chmod +x /usr/local/bin/testing_device
	@echo "Deployed $(TARGET) to /usr/local/bin/testing_device"

# Khởi động lại dịch vụ systemd
restart_service:
	sudo systemctl daemon-reload
	sudo systemctl restart testing_device.service
	@echo "Restarted testing_device.service"

# Clean up
clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

# Phony targets
.PHONY: all clean stop_service deploy restart_service