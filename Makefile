CC=gcc
CFLAGS=-I./include -Wall -Wextra -g -fPIC
LDFLAGS=-pthread -lcjson -ldl
OBJDIR=obj
BINDIR=bin
PLUGINDIR=plugins
TARGET=$(BINDIR)/testing_device

# Source files and object files
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))

# Plugin source files and shared objects
PLUGIN_SRC=$(wildcard plugins/*.c)
PLUGIN_SO=$(patsubst plugins/%.c,$(PLUGINDIR)/%.so,$(PLUGIN_SRC))

# Create directories
$(shell mkdir -p $(OBJDIR) $(BINDIR) $(PLUGINDIR))

# Build everything
all: $(TARGET) $(PLUGIN_SO)

# Link main executable
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile core sources
$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build plugins as shared libraries
$(PLUGINDIR)/%.so: plugins/%.c
	$(CC) $(CFLAGS) -shared -o $@ $< -Wl,-soname,$@

# Create test structure
install: $(TARGET) $(PLUGIN_SO)
	mkdir -p config var/input var/results var/log
	@if [ ! -f config/config.json ]; then \
		echo "Creating sample config file..."; \
		echo '{"test_cases": [{"action": "ping", "attributes": [{"public_attr_name": "host", "private_attr_name": "host", "attr_type": "string", "attr_execute": "yes"}], "input_params": {"host": "8.8.8.8"}}]}' > config/config.json; \
	fi

# Clean objects
clean:
	rm -rf $(OBJDIR)/*.o

# Clean everything
distclean: clean
	rm -rf $(TARGET) $(PLUGIN_SO)

.PHONY: all clean distclean install