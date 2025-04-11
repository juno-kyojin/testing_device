#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#include <stdint.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include "tc.h"

// Plugin API version to ensure compatibility
#define PLUGIN_API_VERSION 1

// Plugin information structure
typedef struct {
    uint32_t api_version;         // Plugin API version
    const char *name;             // Plugin name
    const char *version;          // Plugin version
    const char *description;      // Plugin description
    const char *author;           // Plugin author
} plugin_info_t;

// Plugin capabilities structure
typedef struct {
    const char *action_name;      // Action name (e.g., "ping", "speedtest")
    const char *node_name;        // Node name for tcapi access
    bool has_input_params;        // Whether plugin accepts input parameters
    bool has_custom_output;       // Whether plugin provides custom output handling
} plugin_capabilities_t;

// Forward declarations
struct plugin_context_s;
typedef struct plugin_context_s plugin_context_t;

// Plugin function signatures
typedef const plugin_info_t* (*plugin_get_info_func_t)(void);
typedef const plugin_capabilities_t* (*plugin_get_capabilities_func_t)(void);
typedef int (*plugin_init_func_t)(plugin_context_t *context);
typedef int (*plugin_cleanup_func_t)(plugin_context_t *context);
typedef int (*plugin_execute_func_t)(const plugin_context_t *context, cJSON *input_params, test_result_info_t *result);

// Plugin context structure
struct plugin_context_s {
    void *handle;                 // Plugin handle (from dlopen)
    const plugin_info_t *info;    // Plugin info
    const plugin_capabilities_t *capabilities;  // Plugin capabilities
    void *user_data;              // User data pointer for plugin use
    
    // Core API functions provided to plugins
    int (*log_message)(int level, const char *format, ...); // Logging function
    int (*get_value)(const char *name, char *value, size_t size); // Get config value
    int (*set_value)(const char *name, const char *value); // Set config value
};

// Export macro for plugin functions
#ifdef __cplusplus
#define PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// Plugin API required functions that each plugin must implement
PLUGIN_EXPORT const plugin_info_t* plugin_get_info(void);
PLUGIN_EXPORT const plugin_capabilities_t* plugin_get_capabilities(void);
PLUGIN_EXPORT int plugin_init(plugin_context_t *context);
PLUGIN_EXPORT int plugin_cleanup(plugin_context_t *context);
PLUGIN_EXPORT int plugin_execute(const plugin_context_t *context, 
                               cJSON *input_params,
                               test_result_info_t *result);

#endif /* PLUGIN_API_H */
