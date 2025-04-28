/**
 * @file parser.h
 * @brief Header file for the test case parser
 *
 * This header file defines the interface for parsing test case files in JSON format.
 * It provides a function to read and parse test case files, extracting test case
 * information such as service names and actions, and storing them in an array of
 * `TestCase` structures. The parser is a key component of the test case execution
 * system, enabling the system to process test case definitions from files.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see parser.c
 * @see types.h
 */
#ifndef PARSER_H
#define PARSER_H

#include "types.h"
#include "cjson/cJSON.h"

/**
 * @def MAX_TEST_CASES
 * @brief Maximum number of test cases that can be parsed from a single file
 *
 * Defines the maximum number of test cases that the parser can handle from a single
 * test case file. Currently set to 100 to prevent excessive memory usage and ensure
 * system stability.
 */
#define MAX_TEST_CASES 100

/**
 * @brief Parse a test case file and extract test case information
 *
 * This function reads a JSON file containing test case definitions, parses the
 * test cases, and stores them in an array of `TestCase` structures. Each test case
 * must specify a `service` (e.g., "ping", "speedtest") and may optionally specify
 * an `action`. The function logs errors if the file cannot be read, the JSON is
 * invalid, or the test case format is incorrect. If parsing fails, an error reason
 * is added to the provided `result_json` object.
 *
 * @param filepath Path to the JSON file containing the test cases (e.g., "config/ping.json").
 * @param test_cases Pointer to an array of `TestCase` structures where the parsed test cases will be stored.
 * @param test_case_count Pointer to an integer where the number of parsed test cases will be stored.
 * @param result_json Pointer to a `cJSON` object where error information will be stored if parsing fails.
 * @return int
 *         - 0 if the test cases were successfully parsed.
 *         - -1 if an error occurred (e.g., file not found, invalid JSON, missing test cases).
 */
int parse_test_cases(const char *filepath, TestCase *test_cases, int *test_case_count, cJSON *result_json);

#endif