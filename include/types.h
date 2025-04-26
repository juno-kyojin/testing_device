/**
 * @file types.h
 * @brief Header file defining common types for the test case execution system
 *
 * This header file defines the core data types and structures used throughout the
 * test case execution system. It includes the `TestCase` structure for representing
 * test case information, the `ActionHandler` function pointer type for handling test
 * actions, and the `ActionDispatch` structure for mapping services to their handlers.
 * These types are used by the action dispatch system to route test case requests to
 * the appropriate handlers.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see action.h
 */
#ifndef TYPES_H
#define TYPES_H

#include "cjson/cJSON.h"

/**
 * @struct TestCase
 * @brief Structure to represent a test case definition
 *
 * This structure holds the information for a single test case, including the service
 * name and an optional action. It is used to store test case definitions parsed from
 * JSON files and passed to handler functions for execution.
 *
 * @var TestCase::service
 *      Name of the service associated with the test case (e.g., "ping", "speedtest").
 *      Maximum length is 50 characters, including null terminator.
 * @var TestCase::action
 *      Optional action to perform for the test case (e.g., "create", "delete").
 *      Maximum length is 50 characters, including null terminator. If no action is
 *      specified, this field is an empty string.
 */
typedef struct {
    char service[50];  // Service (ping, speedtest, etc.)
    char action[50];   // Action (create, delete, etc.)
} TestCase;

/**
 * @typedef ActionHandler
 * @brief Function pointer type for handling test case actions
 *
 * Defines a function pointer type for handler functions that process test case actions.
 * Each handler function takes a `TestCase` structure, the path to the test case file,
 * the index of the test case, and a `cJSON` array to store the results.
 *
 * @param test_case Pointer to the `TestCase` structure containing the service and action.
 * @param filepath Path to the JSON file containing the test case definitions.
 * @param index Zero-based index of the test case within the JSON file's test case array.
 * @param result_array Pointer to a `cJSON` array where the test case results will be stored.
 * @return None
 */
typedef void (*ActionHandler)(TestCase *, const char *, int, cJSON *);

/**
 * @struct ActionDispatch
 * @brief Structure for the action dispatch table
 *
 * This structure represents an entry in the action dispatch table, mapping a service
 * name to its corresponding handler function. It is used by the action dispatch system
 * to route test case requests to the appropriate handlers.
 *
 * @var ActionDispatch::service
 *      Name of the service (e.g., "ping", "speedtest").
 *      Maximum length is 50 characters, including null terminator.
 * @var ActionDispatch::handler
 *      Function pointer to the handler for this service, of type `ActionHandler`.
 */
typedef struct {
    char service[50];
    ActionHandler handler;
} ActionDispatch;

#endif