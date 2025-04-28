/**
 * @file parser.c
 * @brief Implementation of the test case parser
 *
 * This file implements the test case parser, which reads and parses test case files
 * in JSON format. It extracts test case information, such as service names and actions,
 * and stores them in an array of `TestCase` structures. The parser is used by the test
 * case execution system to process test case definitions from files and prepare them
 * for execution.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see parser.h
 * @see file_process.h
 * @see log.h
 * @see cjson/cJSON.h
 */
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "file_process.h"
#include "log.h"
#include "cjson/cJSON.h"

int parse_test_cases(const char *filepath, TestCase *test_cases, int *test_case_count, cJSON *result_json) {
    // Read JSON file
    char *json_data;
    size_t json_size;
    if (read_file(filepath, &json_data, &json_size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read %s", filepath);
        cJSON_AddStringToObject(result_json, "fail_reason", "Failed to read file");
        return -1;
    }

    // Kiểm tra nếu file rỗng
    if (json_size == 0) {
        log_message(LOG_LVL_ERROR, "File %s is empty", filepath);
        cJSON_AddStringToObject(result_json, "fail_reason", "Empty file");
        free(json_data);
        return -1;
    }

    // Parse JSON
    cJSON *json = cJSON_Parse(json_data);
    if (!json) {
        log_message(LOG_LVL_ERROR, "Failed to parse %s: %s", filepath, cJSON_GetErrorPtr());
        cJSON_AddStringToObject(result_json, "fail_reason", "Invalid JSON format");
        free(json_data);
        return -1;
    }

    // Get the test_cases array
    cJSON *test_cases_json = cJSON_GetObjectItem(json, "test_cases");
    if (!cJSON_IsArray(test_cases_json)) {
        log_message(LOG_LVL_ERROR, "No 'test_cases' array found in %s", filepath);
        cJSON_AddStringToObject(result_json, "fail_reason", "No test cases array found");
        cJSON_Delete(json);
        free(json_data);
        return -1;
    }

    // Parse each test case
    int count = 0;
    cJSON *test_case_json;
    cJSON_ArrayForEach(test_case_json, test_cases_json) {
        cJSON *service_json = cJSON_GetObjectItem(test_case_json, "service");
        cJSON *action_json = cJSON_GetObjectItem(test_case_json, "action");

        if (!cJSON_IsString(service_json)) {
            log_message(LOG_LVL_ERROR, "Invalid or missing 'service' in test case");
            continue;
        }

        if (count >= MAX_TEST_CASES) {
            log_message(LOG_LVL_ERROR, "Too many test cases, skipping");
            break;
        }

        strncpy(test_cases[count].service, service_json->valuestring, sizeof(test_cases[count].service) - 1);
        test_cases[count].service[sizeof(test_cases[count].service) - 1] = '\0';

        if (action_json && cJSON_IsString(action_json)) {
            strncpy(test_cases[count].action, action_json->valuestring, sizeof(test_cases[count].action) - 1);
            test_cases[count].action[sizeof(test_cases[count].action) - 1] = '\0';
        } else {
            test_cases[count].action[0] = '\0';
        }

        count++;
    }

    *test_case_count = count;

    log_message(LOG_LVL_DEBUG, "Successfully parsed %d test cases from %s", count, filepath);

    cJSON_Delete(json);
    free(json_data);
    return 0;
}