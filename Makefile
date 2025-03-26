CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
LDFLAGS = -pthread -lz

SRC_DIR = src
BUILD_DIR = build
TARGET = test_program

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean