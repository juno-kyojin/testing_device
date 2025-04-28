/**
 * @file test_case_handler.h
 * @brief Header file for test case handling utilities
 *
 * This header file defines the interface for test case handling utilities.
 * It provides a function to handle a test case file by parsing its contents,
 * executing each test case, writing the results to a JSON file, and moving the file
 * to the `processed` directory. These utilities are used in the test case execution
 * system to manage test case files.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see test_case_handler.c
 * @see types.h
 * @see cjson/cJSON.h
 */
#ifndef TEST_CASE_HANDLER_H
#define TEST_CASE_HANDLER_H

#include "types.h"
#include "cjson/cJSON.h"

/**
 * @brief Handle a single test case file
 *
 * This function handles a test case file by parsing its contents, executing each
 * test case, and writing the results to a JSON file. After processing, the file is
 * moved to the `processed` directory to prevent reprocessing. If the `processed`
 * directory does not exist, it is created automatically with appropriate permissions.
 *
 * @param filepath Path to the test case file to process (e.g., "config/ping.json").
 * @return None
 */
void processTestCaseFile(const char *filepath);

#endif /* TEST_CASE_HANDLER_H */