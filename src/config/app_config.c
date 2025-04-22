#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config/app_config.h"
#include "core/log.h"
#include "cjson/cJSON.h"
#include "utils/file_process.h"

// Biến toàn cục để lưu cấu hình thiết bị
DeviceConfig device_config = {0};

int app_config_init(void) {
    char *config_data;
    size_t config_size;

    // Khởi tạo logger
    init_logger();

    // Đọc file config.json
    if (read_file("config/config.json", &config_data, &config_size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read config.json");
        return -1;
    }

    // Parse dữ liệu JSON
    cJSON *config = cJSON_Parse(config_data);
    if (!config) {
        log_message(LOG_LVL_ERROR, "Error parsing config.json: %s", cJSON_GetErrorPtr());
        free(config_data);
        return -1;
    }

    // Lấy log_level từ config.json
    cJSON *log_level = cJSON_GetObjectItem(config, "log_level");
    if (log_level && cJSON_IsNumber(log_level)) {
        set_log_level(log_level->valueint);
        log_message(LOG_LVL_DEBUG, "Set log level to %d", log_level->valueint);
    }

    // Lấy log_file từ config.json
    cJSON *log_file = cJSON_GetObjectItem(config, "log_file");
    if (log_file && cJSON_IsString(log_file)) {
        set_log_file(log_file->valuestring);
        log_message(LOG_LVL_DEBUG, "Set log file to %s", log_file->valuestring);
    }

    // Lấy cấu trúc test case từ instruction
    cJSON *instructions_json = cJSON_GetObjectItem(config, "instruction");
    if (!cJSON_IsArray(instructions_json)) {
        log_message(LOG_LVL_ERROR, "No 'instruction' array found in config.json");
        cJSON_Delete(config);
        free(config_data);
        return -1;
    }

    // Cấp phát bộ nhớ cho danh sách instruction
    device_config.actions.field_count = cJSON_GetArraySize(instructions_json);
    device_config.actions.fields = (KeyValue *)malloc(device_config.actions.field_count * sizeof(KeyValue));
    if (!device_config.actions.fields) {
        log_message(LOG_LVL_ERROR, "Memory allocation failed for instructions");
        cJSON_Delete(config);
        free(config_data);
        return -1;
    }

    // Parse từng instruction
    for (int i = 0; i < device_config.actions.field_count; i++) {
        cJSON *instr_json = cJSON_GetArrayItem(instructions_json, i);
        cJSON *action = cJSON_GetObjectItem(instr_json, "action");

        if (action && cJSON_IsString(action)) {
            // Lưu tên action làm key
            strncpy(device_config.actions.fields[i].key, action->valuestring, sizeof(device_config.actions.fields[i].key) - 1);
            device_config.actions.fields[i].key[sizeof(device_config.actions.fields[i].key) - 1] = '\0';

            // Lưu toàn bộ cấu trúc instruction dưới dạng chuỗi JSON
            char *instr_str = cJSON_PrintUnformatted(instr_json);
            if (instr_str) {
                strncpy(device_config.actions.fields[i].value, instr_str, sizeof(device_config.actions.fields[i].value) - 1);
                device_config.actions.fields[i].value[sizeof(device_config.actions.fields[i].value) - 1] = '\0';
                free(instr_str);
            }
        }
    }

    // Giải phóng tài nguyên
    cJSON_Delete(config);
    free(config_data);
    log_message(LOG_LVL_DEBUG, "Loaded %d instructions from config.json", device_config.actions.field_count);
    return 0;
}

void app_config_cleanup(void) {
    // Dọn dẹp cấu hình thiết bị
    if (device_config.actions.fields) {
        free(device_config.actions.fields);
        device_config.actions.fields = NULL;
        device_config.actions.field_count = 0;
    }

    // Dọn dẹp logger
    cleanup_logger();
}