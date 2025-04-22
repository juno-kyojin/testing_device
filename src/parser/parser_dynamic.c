#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser/parser_dynamic.h"
#include "cjson/cJSON.h"
#include "core/log.h"

void parse_dynamic_fields(cJSON *instr_json, Instruction *instr) {
    // Parse attributes
    cJSON *attributes = cJSON_GetObjectItem(instr_json, "attributes");
    if (!cJSON_IsArray(attributes)) {
        log_message(LOG_LVL_WARN, "No attributes array found for action %s", instr->action);
        instr->attr_count = 0;
        instr->attributes = NULL;
    } else {
        // Cấp phát bộ nhớ cho attributes
        instr->attr_count = cJSON_GetArraySize(attributes);
        instr->attributes = (KeyValue *)malloc(instr->attr_count * sizeof(KeyValue));
        if (!instr->attributes) {
            log_message(LOG_LVL_ERROR, "Memory allocation failed for attributes in action %s", instr->action);
            instr->attr_count = 0;
            return;
        }

        // Parse từng attribute
        for (int i = 0; i < instr->attr_count; i++) {
            cJSON *attr = cJSON_GetArrayItem(attributes, i);
            cJSON *public_attr_name = cJSON_GetObjectItem(attr, "public_attr_name");
            cJSON *value = cJSON_GetObjectItem(attr, "attr_value");
            cJSON *default_value = cJSON_GetObjectItem(attr, "attr_default_value");

            if (public_attr_name && cJSON_IsString(public_attr_name)) {
                strncpy(instr->attributes[i].key, public_attr_name->valuestring, sizeof(instr->attributes[i].key) - 1);
                instr->attributes[i].key[sizeof(instr->attributes[i].key) - 1] = '\0';
            } else {
                instr->attributes[i].key[0] = '\0';
            }

            // Ưu tiên attr_value, nếu rỗng thì dùng attr_default_value
            if (value && cJSON_IsString(value) && strlen(value->valuestring) > 0) {
                strncpy(instr->attributes[i].value, value->valuestring, sizeof(instr->attributes[i].value) - 1);
                instr->attributes[i].value[sizeof(instr->attributes[i].value) - 1] = '\0';
            } else if (default_value && cJSON_IsString(default_value)) {
                strncpy(instr->attributes[i].value, default_value->valuestring, sizeof(instr->attributes[i].value) - 1);
                instr->attributes[i].value[sizeof(instr->attributes[i].value) - 1] = '\0';
            } else {
                instr->attributes[i].value[0] = '\0';
            }
        }
    }

    // Parse entry_attributes (nếu có, như trong traceroute)
    cJSON *entry_attributes = cJSON_GetObjectItem(instr_json, "entry_attributes");
    if (!cJSON_IsArray(entry_attributes)) {
        instr->entry_attr_count = 0;
        instr->entry_attributes = NULL;
    } else {
        // Cấp phát bộ nhớ cho entry_attributes
        instr->entry_attr_count = cJSON_GetArraySize(entry_attributes);
        instr->entry_attributes = (KeyValue *)malloc(instr->entry_attr_count * sizeof(KeyValue));
        if (!instr->entry_attributes) {
            log_message(LOG_LVL_ERROR, "Memory allocation failed for entry_attributes in action %s", instr->action);
            instr->entry_attr_count = 0;
            return;
        }

        // Parse từng entry_attribute
        for (int i = 0; i < instr->entry_attr_count; i++) {
            cJSON *attr = cJSON_GetArrayItem(entry_attributes, i);
            cJSON *public_attr_name = cJSON_GetObjectItem(attr, "public_attr_name");
            cJSON *value = cJSON_GetObjectItem(attr, "attr_value");
            cJSON *default_value = cJSON_GetObjectItem(attr, "attr_default_value");

            if (public_attr_name && cJSON_IsString(public_attr_name)) {
                strncpy(instr->entry_attributes[i].key, public_attr_name->valuestring, sizeof(instr->entry_attributes[i].key) - 1);
                instr->entry_attributes[i].key[sizeof(instr->entry_attributes[i].key) - 1] = '\0';
            } else {
                instr->entry_attributes[i].key[0] = '\0';
            }

            // Ưu tiên attr_value, nếu rỗng thì dùng attr_default_value
            if (value && cJSON_IsString(value) && strlen(value->valuestring) > 0) {
                strncpy(instr->entry_attributes[i].value, value->valuestring, sizeof(instr->entry_attributes[i].value) - 1);
                instr->entry_attributes[i].value[sizeof(instr->entry_attributes[i].value) - 1] = '\0';
            } else if (default_value && cJSON_IsString(default_value)) {
                strncpy(instr->entry_attributes[i].value, default_value->valuestring, sizeof(instr->entry_attributes[i].value) - 1);
                instr->entry_attributes[i].value[sizeof(instr->entry_attributes[i].value) - 1] = '\0';
            } else {
                instr->entry_attributes[i].value[0] = '\0';
            }
        }
    }
}