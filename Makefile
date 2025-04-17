CC = gcc
CFLAGS = -Iinclude -pthread
SRC = src/main.c src/app_config.c src/file_process.c src/log.c src/parser/parser.c src/parser/parser_hardcode.c src/parser/parser_dynamic.c src/test_execute/test_executor.c src/wan.c
OBJ = $(SRC:.c=.o)
EXEC = build/testing_device

all: $(EXEC)

$(EXEC): $(OBJ)
    $(CC) $(OBJ) -o $(EXEC)

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(OBJ) $(EXEC)

.PHONY: all clean