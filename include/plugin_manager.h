#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "plugin_api.h"
#include "tc.h"
#include <cjson/cJSON.h>

/**
 * Initialize plugin manager
 * 
 * @return 0 on success, -1 on error
 */
int plugin_manager_init(void);

/**
 * Load a plugin from file
 * 
 * @param filename Path to plugin file (.so)
 * @return 0 on success, -1 on error
 */
int plugin_load(const char *filename);

/**
 * Discover and load plugins from directory
 * 
 * @param directory Path to plugins directory
 * @return Number of plugins loaded, -1 on error
 */
int plugin_discover(const char *directory);

/**
 * Execute plugin by action name
 * 
 * @param action Action name to execute
 * @param input_params Input parameters as JSON
 * @param result Test result structure to be filled
 * @return 0 on success, -1 on error
 */
int plugin_execute_by_action(const char *action, cJSON *input_params, test_result_info_t *result);

/**
 * Cleanup all plugins
 */
void plugin_manager_cleanup(void);

#endif /* PLUGIN_MANAGER_H */
