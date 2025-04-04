CC=gcc
CFLAGS=-I./include -Wall -Wextra -g
LDFLAGS=-pthread -lcjson
OBJDIR=obj
BINDIR=bin
TARGET=$(BINDIR)/testing_device

# Source files và object files
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))

# Tạo thư mục
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Build chính
all: $(TARGET)

# Link bản thực thi cuối cùng
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Biên dịch từng file source thành file object
$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Tạo cấu trúc thư mục config và mẫu cấu hình
install: $(TARGET)
	mkdir -p config var/input var/results var/log
	@if [ ! -f config/config.json ]; then \
		echo "Creating sample config file..."; \
		echo '{"test_cases": [{"action": "ping", "attributes": [{"public_attr_name": "host", "private_attr_name": "host", "attr_type": "string", "attr_execute": "yes"}], "input_params": {"host": "8.8.8.8"}}]}' > config/config.json; \
	fi

# Dọn dẹp các file tạm
clean:
	rm -rf $(OBJDIR)/*.o $(TARGET)

# Xóa hoàn toàn build
distclean: clean
	rm -rf $(OBJDIR) $(BINDIR)

# Phần .PHONY giúp make biết các targets này không phải là tên file
.PHONY: all clean distclean install