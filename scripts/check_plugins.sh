#!/bin/bash

PLUGIN_DIR="/home/tobie/testing_device/plugins"

# Màu sắc cho output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Tạo thư mục log nếu chưa tồn tại
mkdir -p /home/tobie/testing_device/var/log

echo "Checking plugins in ${PLUGIN_DIR}..."

# Kiểm tra thư mục plugins tồn tại
if [ ! -d "$PLUGIN_DIR" ]; then
    echo -e "${RED}Plugin directory does not exist!${NC}"
    exit 1
fi

# Kiểm tra các file .so
for plugin in "$PLUGIN_DIR"/*.so; do
    if [ -f "$plugin" ]; then
        echo -e "${YELLOW}Checking plugin: $(basename "$plugin")${NC}"
        
        # Kiểm tra file có thể đọc và thực thi
        if [ ! -r "$plugin" ] || [ ! -x "$plugin" ]; then
            echo -e "  ${RED}Error: Plugin doesn't have proper permissions${NC}"
            chmod 755 "$plugin"
            echo -e "  ${GREEN}Fixed permissions${NC}"
        fi
        
        # Kiểm tra định dạng ELF
        if ! file "$plugin" | grep -q "ELF"; then
            echo -e "  ${RED}Error: Not a valid ELF shared library${NC}"
            continue
        fi
        
        # Kiểm tra dependencies
        echo "  Checking dependencies:"
        if ! ldd "$plugin" &> /dev/null; then
            echo -e "  ${RED}Error: ldd failed on this plugin${NC}"
            continue
        fi
        
        missing_deps=$(ldd "$plugin" | grep "not found")
        if [ -n "$missing_deps" ]; then
            echo -e "  ${RED}Error: Missing dependencies:${NC}"
            echo "$missing_deps"
            continue
        fi
        
        # Kiểm tra symbol register_plugin
        if ! nm -D "$plugin" | grep -q "register_plugin"; then
            echo -e "  ${RED}Error: Missing 'register_plugin' symbol${NC}"
            continue
        fi
        
        echo -e "  ${GREEN}Plugin is valid!${NC}"
    fi
done

echo "Plugin check completed!"

# Gợi ý chạy chương trình
echo -e "\n${GREEN}To run with plugins, use:${NC}"
echo -e "USE_PLUGINS=1 ./bin/testing_device ./config/config1.json"
echo -e "\n${GREEN}For debugging plugin issues, use:${NC}"
echo -e "USE_PLUGINS=1 gdb --args ./bin/testing_device ./config/config1.json"
