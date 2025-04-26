/**
 * @file action.h
 * @brief Header file for the action dispatch system
 *
 * This header file defines the interface for the action dispatch mechanism,
 * which routes service requests to their respective handler functions.
 * It provides functions to register services and execute actions based on
 * test case information. The action dispatch system is a core component of
 * the test case execution framework, enabling dynamic dispatching of test actions.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see action.c
 */

#ifndef ACTION_H
#define ACTION_H

#include "types.h"
#include "cjson/cJSON.h"

/**
 * @brief Execute an action based on the test case information
 *
 * This function looks up the appropriate handler for the service specified in
 * the test case using the action dispatch table and executes it. If no handler
 * is found for the specified service, an error is logged, and no action is taken.
 * The results of the test case execution are appended to the provided JSON array.
 *
 * @param test_case Pointer to the `TestCase` structure containing the service name
 *                  (e.g., "ping", "speedtest") and optional action information.
 * @param filepath Path to the JSON file containing the test case definitions.
 * @param index Zero-based index of the test case within the JSON file's test case array.
 * @param result_array Pointer to a `cJSON` array where the test case results will be stored.
 * @return None
 */
void execute_action(TestCase *test_case, const char *filepath, int index, cJSON *result_array);

/**
 * @brief Register a service handler with the action dispatch system
 *
 * Associates a service name with its handler function in the action dispatch table.
 * The service name and handler are stored in the dispatch table, and the registration
 * count is incremented. If the table is full, an error is logged.
 *
 * @param service Name of the service to register (e.g., "ping", "speedtest"). Must be a null-terminated string.
 * @param handler Function pointer to the handler for this service. The handler must match the `ActionHandler` signature.
 * @return None
 */
void register_service(const char *service, void (*handler)(TestCase *, const char *, int, cJSON *));

#endif