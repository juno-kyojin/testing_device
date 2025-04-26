/**
 * @file action.c
 * @brief Implementation of the action dispatch system for test case execution
 *
 * This file implements the action dispatch mechanism, which routes service
 * requests (e.g., "ping", "speedtest") to their respective handler functions.
 * It is a core component of the test case execution system, enabling dynamic
 * dispatching of test actions based on the service specified in the test case.
 * The dispatch table maps service names to their handlers, and actions are executed
 * by looking up the appropriate handler for a given service.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see action.h
 * @see action_registry.h
 */
#include <stdio.h>
#include <string.h>
#include "action.h"
#include "action_registry.h"
#include "log.h"

/**
 * @def MAX_SERVICES
 * @brief Maximum number of services that can be registered in the action dispatch table
 *
 * Defines the maximum number of services that can be registered in the system.
 * Currently set to 100 to limit memory usage and prevent overflow of the dispatch table.
 */
#define MAX_SERVICES 100

/**
 * @var action_table
 * @brief Action dispatch table mapping service names to handler functions
 *
 * This static array stores the mapping of service names (e.g., "ping", "speedtest")
 * to their corresponding handler functions. Each entry is of type `ActionDispatch`,
 * which contains a service name and a function pointer (`ActionHandler`).
 * The table has a fixed size defined by `MAX_SERVICES`.
 */
static ActionDispatch action_table[MAX_SERVICES];

/**
 * @var action_count
 * @brief Current number of services registered in the action dispatch table
 *
 * This static variable keeps track of the number of services currently registered
 * in the `action_table`. It is incremented each time a new service is registered
 * via `register_service()`.
 */
static int action_count = 0;

void register_service(const char *service, ActionHandler handler) {
    if (action_count >= MAX_SERVICES) {
        log_message(LOG_LVL_ERROR, "Action dispatch table is full");
        return;
    }
    strncpy(action_table[action_count].service, service, sizeof(action_table[action_count].service) - 1);
    action_table[action_count].service[sizeof(action_table[action_count].service) - 1] = '\0';
    action_table[action_count].handler = handler;
    action_count++;
}

/**
 * @brief Look up a handler for a service in the action dispatch table
 *
 * This function searches the `action_table` for a service with the given name
 * and returns its associated handler function. If no matching service is found,
 * it returns NULL.
 *
 * @param service Name of the service to look up (e.g., "ping", "speedtest"). Must be a null-terminated string.
 * @return ActionHandler
 *         - Function pointer to the handler if the service is found.
 *         - NULL if no matching service is found in the dispatch table.
 */
static ActionHandler get_action_handler(const char *service) {
    for (int i = 0; i < action_count; i++) {
        if (strcmp(action_table[i].service, service) == 0) {
            return action_table[i].handler;
        }
    }
    return NULL;
}

void execute_action(TestCase *test_case, const char *filepath, int index, cJSON *result_array) {
    log_message(LOG_LVL_DEBUG, "Executing action: %s (service: %s, action: %s)", 
                test_case->action[0] ? test_case->action : test_case->service, 
                test_case->service, 
                test_case->action[0] ? test_case->action : "default");

    ActionHandler handler = get_action_handler(test_case->service);
    if (handler) {
        handler(test_case, filepath, index, result_array);
    } else {
        log_message(LOG_LVL_ERROR, "No handler found for service: %s", test_case->service);
    }
}