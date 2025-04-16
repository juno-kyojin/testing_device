#include "plugin_manager.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

// Số lượng plugin tối đa có thể nạp
#define MAX_PLUGINS 20

// Mảng lưu thông tin các plugin đã nạp
static plugin_info_t loaded_plugins[MAX_PLUGINS];
static int num_loaded_plugins = 0;

// Thêm ở đầu file
#include <setjmp.h>
#include <signal.h>

static jmp_buf jmpbuf;

static void signal_handler(int sig) {
    if (sig == SIGSEGV) {
        longjmp(jmpbuf, 1);
    }
}

// Hàm tạo cấu trúc plugin_info rỗng
plugin_info_t create_empty_plugin(void) {
    plugin_info_t empty_plugin;
    memset(&empty_plugin, 0, sizeof(plugin_info_t));
    
    // Khởi tạo các trường mới
    empty_plugin.original_action[0] = '\0';
    
    return empty_plugin;
}

// Hàm khởi tạo hệ thống plugin - cần cập nhật để khởi tạo original_action
int plugin_manager_init(void) {
    log_message(LOG_LVL_DEBUG, "Initializing plugin manager");
    
    // Reset mảng plugin
    memset(loaded_plugins, 0, sizeof(loaded_plugins));
    for (int i = 0; i < MAX_PLUGINS; i++) {
        loaded_plugins[i].original_action[0] = '\0';
    }
    num_loaded_plugins = 0;
    
    return 0;
}

int load_plugins(const char *plugin_dir) {
    DIR *dir;
    struct dirent *entry;
    int loaded_count = 0;
    
    // Thêm kiểm tra NULL pointer
    if (!plugin_dir) {
        log_message(LOG_LVL_ERROR, "Plugin directory path is NULL");
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Loading plugins from directory: %s", plugin_dir);
    
    dir = opendir(plugin_dir);
    if (!dir) {
        log_message(LOG_LVL_ERROR, "Failed to open plugin directory: %s", plugin_dir);
        return -1;
    }
    
    // Duyệt qua các file trong thư mục plugin
    while ((entry = readdir(dir)) != NULL) {
        // Chỉ xử lý các file .so
        if (strstr(entry->d_name, ".so") == NULL) {
            continue;
        }
        
        char plugin_path[512];
        snprintf(plugin_path, sizeof(plugin_path), "%s/%s", plugin_dir, entry->d_name);
        
        log_message(LOG_LVL_DEBUG, "Found plugin file: %s", plugin_path);
        
        // Thử nạp file dynamic library với xử lý lỗi chi tiết
        dlerror(); // Xóa bất kỳ lỗi nào còn tồn tại
        void *handle = dlopen(plugin_path, RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            const char *error = dlerror();
            log_message(LOG_LVL_ERROR, "Failed to load plugin %s: %s", plugin_path, error ? error : "Unknown error");
            continue;
        }
        
        // Kiểm tra lỗi dlsym
        dlerror();
        
        // Tìm hàm register_plugin - đây là entry point của plugin
        typedef plugin_info_t (*register_plugin_fn)(void);
        register_plugin_fn register_plugin = (register_plugin_fn)dlsym(handle, "register_plugin");
        
        const char *dlsym_error = dlerror();
        if (dlsym_error) {
            log_message(LOG_LVL_ERROR, "Plugin %s error loading register_plugin: %s", 
                        plugin_path, dlsym_error);
            dlclose(handle);
            continue;
        }
        
        if (!register_plugin) {
            log_message(LOG_LVL_ERROR, "Plugin %s register_plugin function is NULL", plugin_path);
            dlclose(handle);
            continue;
        }
        
        // Gọi hàm đăng ký plugin để lấy thông tin plugin
        log_message(LOG_LVL_DEBUG, "Calling register_plugin for %s", plugin_path);
        plugin_info_t plugin_info;
        memset(&plugin_info, 0, sizeof(plugin_info_t));
        
        // Bảo vệ khỏi segfault trong plugin
        if (setjmp(jmpbuf) == 0) {
            // Thiết lập signal handler
            struct sigaction sa, old_sa;
            memset(&sa, 0, sizeof(sa));
            sa.sa_handler = signal_handler;
            sigaction(SIGSEGV, &sa, &old_sa);
            
            // Gọi hàm register_plugin của plugin
            plugin_info = register_plugin();
            
            // Khôi phục signal handler
            sigaction(SIGSEGV, &old_sa, NULL);
        } else {
            log_message(LOG_LVL_ERROR, "Segmentation fault occurred during plugin registration: %s", plugin_path);
            dlclose(handle);
            continue;
        }
        
        // Lưu handle vào cấu trúc plugin_info
        plugin_info.handle = handle;
        
        // Kiểm tra các thông tin bắt buộc
        if (strlen(plugin_info.name) == 0) {
            log_message(LOG_LVL_ERROR, "Plugin %s has empty name", plugin_path);
            dlclose(handle);
            continue;
        }
        
        if (strlen(plugin_info.action_name) == 0) {
            log_message(LOG_LVL_ERROR, "Plugin %s has empty action_name", plugin_path);
            dlclose(handle);
            continue;
        }
        
        if (plugin_info.execute_test == NULL) {
            log_message(LOG_LVL_ERROR, "Plugin %s has NULL execute_test function", plugin_path);
            dlclose(handle);
            continue;
        }
        
        // Initialize original_action field
        strncpy(plugin_info.original_action, plugin_info.action_name, sizeof(plugin_info.original_action) - 1);
        plugin_info.original_action[sizeof(plugin_info.original_action) - 1] = '\0';
        
        // Kiểm tra nếu đã đạt số lượng plugin tối đa
        if (num_loaded_plugins >= MAX_PLUGINS) {
            log_message(LOG_LVL_ERROR, "Maximum number of plugins (%d) reached, cannot load %s",
                        MAX_PLUGINS, plugin_path);
            dlclose(handle);
            break;
        }
        
        // Kiểm tra xem action_name có bị trùng không
        bool action_exists = false;
        for (int i = 0; i < num_loaded_plugins; i++) {
            if (strcmp(loaded_plugins[i].action_name, plugin_info.action_name) == 0) {
                action_exists = true;
                break;
            }
        }
        
        if (action_exists) {
            // Thay vì lỗi, chỉ hiển thị cảnh báo
            log_message(LOG_LVL_WARN, "Plugin %s có action_name trùng với plugin đã nạp: %s. Sẽ đánh dấu là biến thể.",
                        plugin_path, plugin_info.action_name);
            
            // Thêm hậu tố để phân biệt với plugin đã nạp
            char variant_name[64];
            int variant_num = 2;
            
            for (int i = 0; i < num_loaded_plugins; i++) {
                if (strstr(loaded_plugins[i].action_name, plugin_info.action_name) == loaded_plugins[i].action_name) {
                    variant_num++;
                }
            }
            
            snprintf(variant_name, sizeof(variant_name), "%s_v%d", plugin_info.action_name, variant_num);
            // Lưu action name gốc trước khi đổi
            strncpy(plugin_info.original_action, plugin_info.action_name, sizeof(plugin_info.original_action) - 1);
            plugin_info.original_action[sizeof(plugin_info.original_action) - 1] = '\0';
            // Đổi tên action
            strncpy(plugin_info.action_name, variant_name, sizeof(plugin_info.action_name) - 1);
            plugin_info.action_name[sizeof(plugin_info.action_name) - 1] = '\0';
            
            // Đổi LOG_LVL_INFO thành LOG_LVL_DEBUG
            log_message(LOG_LVL_DEBUG, "Plugin được đổi tên thành: %s", variant_name);
        }
        
        // Khởi tạo plugin nếu có hàm initialize
        if (plugin_info.initialize != NULL) {
            // Sử dụng setjmp/longjmp để bắt lỗi segmentation fault
            if (setjmp(jmpbuf) == 0) {
                // Thiết lập bộ xử lý tín hiệu
                struct sigaction sa, old_sa;
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = 0;
                sa.sa_handler = SIG_DFL;
                sigaction(SIGSEGV, &sa, &old_sa);
                
                int init_result = plugin_info.initialize();
                
                // Khôi phục bộ xử lý tín hiệu
                sigaction(SIGSEGV, &old_sa, NULL);
                
                if (init_result != 0) {
                    log_message(LOG_LVL_ERROR, "Failed to initialize plugin %s", plugin_path);
                    dlclose(handle);
                    continue;
                }
            } else {
                log_message(LOG_LVL_ERROR, "Segmentation fault occurred while initializing plugin %s", plugin_path);
                dlclose(handle);
                continue;
            }
        }
        
        // Thêm plugin vào danh sách
        loaded_plugins[num_loaded_plugins++] = plugin_info;
        loaded_count++;
        
        log_message(LOG_LVL_DEBUG, "Successfully loaded plugin: %s (action: %s, version: %s)",
                    plugin_info.name, plugin_info.action_name, plugin_info.version);
    }
    
    closedir(dir);
    log_message(LOG_LVL_DEBUG, "Loaded %d plugins successfully", loaded_count);
    
    return loaded_count;
}

plugin_info_t* find_plugin_by_action(const char *action_name) {
    if (!action_name) {
        return NULL;
    }
    
    for (int i = 0; i < num_loaded_plugins; i++) {
        if (strcmp(loaded_plugins[i].action_name, action_name) == 0) {
            return &loaded_plugins[i];
        }
    }
    
    return NULL;
}

plugin_info_t* find_all_plugins_by_action(const char *action_name, int *count) {
    if (!action_name || !count) {
        return NULL;
    }
    
    *count = 0;
    plugin_info_t *result = NULL;
    
    // Đếm số lượng plugin phù hợp
    for (int i = 0; i < num_loaded_plugins; i++) {
        if (strcmp(loaded_plugins[i].action_name, action_name) == 0 || 
            strcmp(loaded_plugins[i].original_action, action_name) == 0) {
            (*count)++;
        }
    }
    
    if (*count == 0) {
        return NULL;
    }
    
    // Cấp phát bộ nhớ
    result = (plugin_info_t*)malloc(sizeof(plugin_info_t) * (*count));
    if (!result) {
        *count = 0;
        return NULL;
    }
    
    // Sao chép các plugin phù hợp
    int idx = 0;
    for (int i = 0; i < num_loaded_plugins; i++) {
        if (strcmp(loaded_plugins[i].action_name, action_name) == 0 || 
            strcmp(loaded_plugins[i].original_action, action_name) == 0) {
            memcpy(&result[idx], &loaded_plugins[i], sizeof(plugin_info_t));
            result[idx].original_action[0] = '\0'; // Initialize original_action field
            idx++;
        }
    }
    
    return result;
}

int plugin_execute_test(const char *action_name, test_case_t *test_case, 
                         test_result_info_t *result, cJSON *input_params) {
    if (!action_name || !test_case || !result) {
        log_message(LOG_LVL_ERROR, "Invalid parameters for plugin_execute_test");
        return -1;
    }
    
    // Tìm plugin tương ứng với action_name
    plugin_info_t *plugin = find_plugin_by_action(action_name);
    if (!plugin) {
        log_message(LOG_LVL_ERROR, "No plugin found for action: %s", action_name);
        return -1;
    }
    
    // Kiểm tra xem plugin có hàm execute_test không
    if (!plugin->execute_test) {
        log_message(LOG_LVL_ERROR, "Plugin %s doesn't have execute_test function", plugin->name);
        return -1;
    }
    
    // Gọi hàm thực thi test của plugin với try-catch để tránh crash
    log_message(LOG_LVL_DEBUG, "Executing test via plugin %s (action: %s)", 
                plugin->name, plugin->action_name);
    
    // Sử dụng setjmp/longjmp để bắt lỗi segmentation fault
    jmp_buf jmpbuf;
    int result_code = -1;
    
    if (setjmp(jmpbuf) == 0) {
        // Thiết lập bộ xử lý tín hiệu
        struct sigaction sa, old_sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = SIG_DFL;
        sigaction(SIGSEGV, &sa, &old_sa);
        
        // Gọi hàm execute_test của plugin
        result_code = plugin->execute_test(test_case, result, input_params);
        
        // Khôi phục bộ xử lý tín hiệu
        sigaction(SIGSEGV, &old_sa, NULL);
    } else {
        log_message(LOG_LVL_ERROR, "Segmentation fault occurred in plugin execution: %s", plugin->name);
        result_code = -1;
    }
    
    return result_code;
}

void plugin_manager_cleanup(void) {
    log_message(LOG_LVL_DEBUG, "Cleaning up plugin manager");
    
    for (int i = 0; i < num_loaded_plugins; i++) {
        // Gọi hàm cleanup của plugin nếu có
        if (loaded_plugins[i].cleanup) {
            loaded_plugins[i].cleanup();
        }
        
        // Giải phóng handle
        if (loaded_plugins[i].handle) {
            dlclose(loaded_plugins[i].handle);
            loaded_plugins[i].handle = NULL;
        }
    }
    
    // Reset mảng plugin
    memset(loaded_plugins, 0, sizeof(loaded_plugins));
    num_loaded_plugins = 0;
}

/**
 * @brief Lấy danh sách các plugin đã nạp
 * 
 * @param count Con trỏ đến biến lưu số lượng plugin
 * @return plugin_info_t* Mảng chứa thông tin các plugin
 */
plugin_info_t* get_loaded_plugins(int *count) {
    if (count) {
        *count = num_loaded_plugins;
    }
    return loaded_plugins;
}
