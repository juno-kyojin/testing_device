CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
LDFLAGS = -pthread -lz

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Tìm tất cả các file nguồn
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Tên chương trình thực thi
TARGET = $(BIN_DIR)/device_test

# Tạo thư mục nếu chưa tồn tại
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) logs results)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all clean