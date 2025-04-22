#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser/parser.h"
#include "cjson/cJSON.h"
#include "utils/file_process.h"
#include "core/log.h"

int parser_data(const char *file_path, TestCase **test_cases, int *test_case_count) {
    char *json_str;
    size_t size;

    // Đọc file
    if (read_file(file_path, &json_str, &size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read test file: %s", file_path);
        return -1;
    }

    // Parse JSON
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        log_message(LOG_LVL_ERROR, "Error parsing JSON: %s", cJSON_GetErrorPtr());
        free(json_str);
        return -1;
    }

    // Lấy mảng test_cases
    cJSON *test_cases_json = cJSON_GetObjectItem(root, "test_cases");
    if (!cJSON_IsArray(test_cases_json)) {
        log_message(LOG_LVL_ERROR, "JSON does not contain 'test_cases' array");
        cJSON_Delete(root);
        free(json_str);
        return -1;
    }

    // Cấp phát bộ nhớ cho danh sách test case
    *test_case_count = cJSON_GetArraySize(test_cases_json);
    *test_cases = (TestCase *)malloc(*test_case_count * sizeof(TestCase));
    if (!*test_cases) {
        log_message(LOG_LVL_ERROR, "Memory allocation failed for test cases");
        cJSON_Delete(root);
        free(json_str);
        return -1;
    }

    // Parse từng test case
    for (int i = 0; i < *test_case_count; i++) {
        cJSON *test_case_json = cJSON_GetArrayItem(test_cases_json, i);
        cJSON *action = cJSON_GetObjectItem(test_case_json, "action");
        if (action && cJSON_IsString(action)) {
            strncpy((*test_cases)[i].action, action->valuestring, sizeof((*test_cases)[i].action) - 1);
            (*test_cases)[i].action[sizeof((*test_cases)[i].action) - 1] = '\0';
        } else {
            log_message(LOG_LVL_WARN, "Invalid or missing action at index %d", i);
            (*test_cases)[i].action[0] = '\0';
        }

        // Parse input_params
        cJSON *input_params = cJSON_GetObjectItem(test_case_json, "input_params");
        if (input_params && cJSON_IsObject(input_params)) {
            // Đếm số phần tử trong input_params
            int param_count = 0;
            cJSON *param;
            cJSON_ArrayForEach(param, input_params) {
                if (cJSON_IsString(param)) {
                    param_count++;
                }
            }
            (*test_cases)[i].param_count = param_count;
            (*test_cases)[i].input_params = (KeyValue *)malloc(param_count * sizeof(KeyValue));
            if (!(*test_cases)[i].input_params) {
                log_message(LOG_LVL_ERROR, "Memory allocation failed for input_params");
                for (int j = 0; j < i; j++) {
                    if ((*test_cases)[j].input_params) free((*test_cases)[j].input_params);
                    if ((*test_cases)[j].attributes) free((*test_cases)[j].attributes);
                }
                free(*test_cases);
                cJSON_Delete(root);
                free(json_str);
                return -1;
            }

            int param_idx = 0;
            cJSON_ArrayForEach(param, input_params) {
                if (cJSON_IsString(param)) {
                    strncpy((*test_cases)[i].input_params[param_idx].key, param->string, sizeof((*test_cases)[i].input_params[param_idx].key) - 1);
                    (*test_cases)[i].input_params[param_idx].key[sizeof((*test_cases)[i].input_params[param_idx].key) - 1] = '\0';
                    strncpy((*test_cases)[i].input_params[param_idx].value, param->valuestring, sizeof((*test_cases)[i].input_params[param_idx].value) - 1);
                    (*test_cases)[i].input_params[param_idx].value[sizeof((*test_cases)[i].input_params[param_idx].value) - 1] = '\0';
                    param_idx++;
                }
            }
        } else {
            (*test_cases)[i].param_count = 0;
            (*test_cases)[i].input_params = NULL;
        }

        // Parse attributes
        cJSON *attributes = cJSON_GetObjectItem(test_case_json, "attributes");
        if (!cJSON_IsArray(attributes)) {
            (*test_cases)[i].attr_count = 0;
            (*test_cases)[i].attributes = NULL;
        } else {
            (*test_cases)[i].attr_count = cJSON_GetArraySize(attributes);
            (*test_cases)[i].attributes = (KeyValue *)malloc((*test_cases)[i].attr_count * sizeof(KeyValue));
            if (!(*test_cases)[i].attributes) {
                log_message(LOG_LVL_ERROR, "Memory allocation failed for attributes");
                for (int j = 0; j < i; j++) {
                    if ((*test_cases)[j].input_params) free((*test_cases)[j].input_params);
                    if ((*test_cases)[j].attributes) free((*test_cases)[j].attributes);
                }
                if ((*test_cases)[i].input_params) free((*test_cases)[i].input_params);
                free(*test_cases);
                cJSON_Delete(root);
                free(json_str);
                return -1;
            }

            for (int j = 0; j < (*test_cases)[i].attr_count; j++) {
                cJSON *attr = cJSON_GetArrayItem(attributes, j);
                cJSON *public_attr_name = cJSON_GetObjectItem(attr, "public_attr_name");
                cJSON *value = cJSON_GetObjectItem(attr, "attr_value");
                cJSON *default_value = cJSON_GetObjectItem(attr, "attr_default_value");

                if (public_attr_name && cJSON_IsString(public_attr_name)) {
                    strncpy((*test_cases)[i].attributes[j].key, public_attr_name->valuestring, sizeof((*test_cases)[i].attributes[j].key) - 1);
                    (*test_cases)[i].attributes[j].key[sizeof((*test_cases)[i].attributes[j].key) - 1] = '\0';
                } else {
                    (*test_cases)[i].attributes[j].key[0] = '\0';
                }

                if (value && cJSON_IsString(value) && strlen(value->valuestring) > 0) {
                    strncpy((*test_cases)[i].attributes[j].value, value->valuestring, sizeof((*test_cases)[i].attributes[j].value) - 1);
                    (*test_cases)[i].attributes[j].value[sizeof((*test_cases)[i].attributes[j].value) - 1] = '\0';
                } else if (default_value && cJSON_IsString(default_value)) {
                    strncpy((*test_cases)[i].attributes[j].value, default_value->valuestring, sizeof((*test_cases)[i].attributes[j].value) - 1);
                    (*test_cases)[i].attributes[j].value[sizeof((*test_cases)[i].attributes[j].value) - 1] = '\0';
                } else {
                    (*test_cases)[i].attributes[j].value[0] = '\0';
                }
            }
        }
    }

    cJSON_Delete(root);
    free(json_str);
    log_message(LOG_LVL_DEBUG, "Successfully parsed %d test cases from %s", *test_case_count, file_path);
    return 0;
}   