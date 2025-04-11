#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/types.h>
#include "plugin_api.h"
#include "log.h"
#include "tc_api.h"  // Add this include for tcapi functions

// Maximum number of plugins to support
#define MAX_PLUGINS 32

// Plugin registry
typedef struct {
    char name[64];                // Plugin name
    plugin_context_t context;     // Plugin context
    plugin_get_info_func_t get_info;
    plugin_get_capabilities_func_t get_capabilities;
    plugin_init_func_t init;
    plugin_cleanup_func_t cleanup;
    plugin_execute_func_t execute;
    bool loaded;                  // Whether plugin is fully loaded
} plugin_registry_t;

// Plugin manager state
static plugin_registry_t plugins[MAX_PLUGINS];
static int plugin_count = 0;

// Forward declarations of helper functions for plugins
static int plugin_log_message(int level, const char *format, ...);
static int plugin_get_value(const char *name, char *value, size_t size);
static int plugin_set_value(const char *name, const char *value);

// Initialize plugin manager
int plugin_manager_init(void) {
    log_message(LOG_LVL_DEBUG, "Initializing plugin manager");
    memset(plugins, 0, sizeof(plugins));
    plugin_count = 0;
    return 0;
}

// Load a plugin from file
int plugin_load(const char *filename) {
    if (plugin_count >= MAX_PLUGINS) {
        log_message(LOG_LVL_ERROR, "Maximum number of plugins reached (%d)", MAX_PLUGINS);
        return -1;
    }
    
    // Open plugin library
    void *handle = dlopen(filename, RTLD_LAZY);
    if (!handle) {
        log_message(LOG_LVL_ERROR, "Failed to load plugin '%s': %s", filename, dlerror());
        return -1;
    }
    
    // Get plugin functions
    plugin_get_info_func_t get_info = (plugin_get_info_func_t)dlsym(handle, "plugin_get_info");
    plugin_get_capabilities_func_t get_capabilities = (plugin_get_capabilities_func_t)dlsym(handle, "plugin_get_capabilities");
    plugin_init_func_t init = (plugin_init_func_t)dlsym(handle, "plugin_init");
    plugin_cleanup_func_t cleanup = (plugin_cleanup_func_t)dlsym(handle, "plugin_cleanup");
    plugin_execute_func_t execute = (plugin_execute_func_t)dlsym(handle, "plugin_execute");
    
    // Check if all required functions are present
    if (!get_info || !get_capabilities || !init || !cleanup || !execute) {
        log_message(LOG_LVL_ERROR, "Plugin '%s' is missing required functions", filename);
        dlclose(handle);
        return -1;
    }
    
    // Get plugin info and check API version
    const plugin_info_t *info = get_info();
    if (!info || info->api_version != PLUGIN_API_VERSION) {
        log_message(LOG_LVL_ERROR, "Plugin '%s' has incompatible API version %d (expected %d)", 
                   filename, info ? info->api_version : 0, PLUGIN_API_VERSION);
        dlclose(handle);
        return -1;
    }
    
    // Get plugin capabilities
    const plugin_capabilities_t *capabilities = get_capabilities();
    if (!capabilities || !capabilities->action_name || !capabilities->node_name) {
        log_message(LOG_LVL_ERROR, "Plugin '%s' has invalid capabilities", filename);
        dlclose(handle);
        return -1;
    }
    
    // Store plugin in registry
    plugin_registry_t *plugin = &plugins[plugin_count];
    strncpy(plugin->name, info->name, sizeof(plugin->name) - 1);
    plugin->name[sizeof(plugin->name) - 1] = '\0';
    
    plugin->context.handle = handle;
    plugin->context.info = info;
    plugin->context.capabilities = capabilities;
    plugin->context.user_data = NULL;
    plugin->context.log_message = plugin_log_message;
    plugin->context.get_value = plugin_get_value;
    plugin->context.set_value = plugin_set_value;
    
    plugin->get_info = get_info;
    plugin->get_capabilities = get_capabilities;
    plugin->init = init;
    plugin->cleanup = cleanup;
    plugin->execute = execute;
    
    // Initialize plugin
    if (init(&plugin->context) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize plugin '%s'", plugin->name);
        dlclose(handle);
        return -1;
    }
    
    plugin->loaded = true;
    log_message(LOG_LVL_INFO, "Loaded plugin '%s' version %s by %s", 
               info->name, info->version, info->author);
    
    plugin_count++;
    return 0;
}

// Discover and load plugins from directory
int plugin_discover(const char *directory) {
    log_message(LOG_LVL_DEBUG, "Discovering plugins in '%s'", directory);
    
    DIR *dir = opendir(directory);
    if (!dir) {
        log_message(LOG_LVL_ERROR, "Failed to open plugin directory '%s'", directory);
        return -1;
    }
    
    struct dirent *entry;
    int count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        // Only load files ending with .so
        size_t len = strlen(entry->d_name);
        if (len < 3 || strcmp(entry->d_name + len - 3, ".so") != 0) {
            continue;
        }
        
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        
        if (plugin_load(path) == 0) {
            count++;
        }
    }
    
    closedir(dir);
    log_message(LOG_LVL_INFO, "Discovered %d plugins in '%s'", count, directory);
    return count;
}

// Find plugin by action name
plugin_registry_t *plugin_find_by_action(const char *action) {
    for (int i = 0; i < plugin_count; i++) {
        if (plugins[i].loaded && 
            strcmp(plugins[i].context.capabilities->action_name, action) == 0) {
            return &plugins[i];
        }
    }
    return NULL;
}

// Execute plugin by action name
int plugin_execute_by_action(const char *action, cJSON *input_params, test_result_info_t *result) {
    plugin_registry_t *plugin = plugin_find_by_action(action);
    if (!plugin) {
        log_message(LOG_LVL_ERROR, "No plugin found for action '%s'", action);
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Executing plugin '%s' for action '%s'", 
               plugin->name, action);
    
    return plugin->execute(&plugin->context, input_params, result);
}

// Cleanup all plugins
void plugin_manager_cleanup(void) {
    log_message(LOG_LVL_DEBUG, "Cleaning up plugin manager");
    
    for (int i = 0; i < plugin_count; i++) {
        if (plugins[i].loaded) {
            // Call plugin cleanup
            plugins[i].cleanup(&plugins[i].context);
            
            // Close plugin handle
            if (plugins[i].context.handle) {
                dlclose(plugins[i].context.handle);
                plugins[i].context.handle = NULL;
            }
            
            plugins[i].loaded = false;
        }
    }
    
    plugin_count = 0;
}

// Helper functions used by plugins

static int plugin_log_message(int level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = log_message_v(level, format, args);
    va_end(args);
    return ret;
}

static int plugin_get_value(const char *name, char *value, size_t size) {
    // Simplified implementation - in a real system, this might access
    // a central configuration repository
    if (!name || !value || size == 0) return -1;
    
    // Example implementation just passes through to tcapi_get
    // This would need to be enhanced to handle plugin-specific configs
    return tcapi_get("Config", "Entry", name, value);
}

static int plugin_set_value(const char *name, const char *value) {
    // Simplified implementation
    if (!name || !value) return -1;
    
    // Example implementation just passes through to tcapi_set
    return tcapi_set("Config", "Entry", name, value);
}
