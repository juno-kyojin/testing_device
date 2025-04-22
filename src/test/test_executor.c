#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test/test_executor.h"
#include "parser/parser.h"
#include "parser/parser_hardcode.h"
#include "parser/parser_dynamic.h"
#include "action/action.h"
#include "test/ping.h"
#include "core/log.h"
#include "cjson/cJSON.h"

// Biến toàn cục từ app_config.c
extern DeviceConfig device_config;

int execute_tests(const char *test_file_path) {
    TestCase *test_cases;
    int test_case_count;

    // Parse file để lấy danh sách test case
    if (parser_data(test_file_path, &test_cases, &test_case_count) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to parse test file: %s", test_file_path);
        return -1;
    }

    // Thực thi từng test case
    for (int i = 0; i < test_case_count; i++) {
        if (test_cases[i].action[0]) {
            // Tìm cấu trúc instruction trong device_config
            int found = 0;
            for (int j = 0; j < device_config.actions.field_count; j++) {
                if (strcmp(device_config.actions.fields[j].key, test_cases[i].action) == 0) {
                    found = 1;
                    // Parse instruction từ chuỗi JSON
                    cJSON *instr_json = cJSON_Parse(device_config.actions.fields[j].value);
                    if (!instr_json) {
                        log_message(LOG_LVL_ERROR, "Failed to parse instruction for action: %s. JSON string: %s", 
                                    test_cases[i].action, device_config.actions.fields[j].value);
                        continue;
                    }

                    Instruction instr;
                    memset(&instr, 0, sizeof(Instruction));

                    // Parse các trường cố định
                    parse_hardcode_fields(instr_json, &instr);

                    // Parse attributes động
                    parse_dynamic_fields(instr_json, &instr);

                    // Thực thi hành động với input_params và attributes
                    execute_action(&instr, &test_cases[i]);

                    // Giải phóng tài nguyên
                    if (instr.attributes) free(instr.attributes);
                    if (instr.entry_attributes) free(instr.entry_attributes);
                    cJSON_Delete(instr_json);
                    break;
                }
            }
            if (!found) {
                log_message(LOG_LVL_ERROR, "Action not found in config: %s", test_cases[i].action);
            }
        }
    }

    // Giải phóng danh sách test case
    for (int i = 0; i < test_case_count; i++) {
        if (test_cases[i].input_params) free(test_cases[i].input_params);
        if (test_cases[i].attributes) free(test_cases[i].attributes);
    }
    free(test_cases);
    return 0;
}