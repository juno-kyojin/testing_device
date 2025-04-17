#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cjson/cJSON.h"
#include "parser.h"
#include "parser_hardcode.h"
#include "parser_dynamic.h"
#include "file_process.h"
#include "log.h"

int parser_data(const char *file_path, Instruction **instructions) {
    char *json_str;
    size_t size;
    if (read_file(file_path, &json_str, &size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read JSON file: %s", file_path);
        return 0;
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        log_message(LOG_LVL_ERROR, "Error parsing JSON: %s", cJSON_GetErrorPtr());
        free(json_str);
        return 0;
    }

    cJSON *instr_array = cJSON_GetObjectItem(root, "instruction");
    if (!cJSON_IsArray(instr_array)) {
        log_message(LOG_LVL_ERROR, "JSON does not contain 'instruction' array");
        cJSON_Delete(root);
        free(json_str);
        return 0;
    }

    int instr_count = cJSON_GetArraySize(instr_array);
    *instructions = (Instruction *)malloc(instr_count * sizeof(Instruction));

    for (int i = 0; i < instr_count; i++) {
        cJSON *instr_json = cJSON_GetArrayItem(instr_array, i);
        Instruction *instr = &(*instructions)[i];

        parse_hardcode_fields(instr_json, instr);
        parse_dynamic_fields(instr_json, instr);
    }

    cJSON_Delete(root);
    free(json_str);
    return instr_count;
}