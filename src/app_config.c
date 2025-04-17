#include <stdio.h>
#include <stdlib.h>
#include "app_config.h"
#include "log.h"
#include "cjson/cJSON.h"
#include "file_process.h"

int app_config_init(void) {
    // Khởi tạo logger
    init_logger();

    // Đọc cấu hình từ file config.json
    char *config_data;
    size_t config_size;
    if (read_file("config/config.json", &config_data, &config_size) == 0) {
        cJSON *config = cJSON_Parse(config_data);
        if (config) {
            cJSON *log_level = cJSON_GetObjectItem(config, "log_level");
            cJSON *log_file = cJSON_GetObjectItem(config, "log_file");

            if (log_level && cJSON_IsNumber(log_level)) {
                set_log_level(log_level->valueint);
                log_message(LOG_LVL_DEBUG, "Set log level to %d", log_level->valueint);
            }
            if (log_file && cJSON_IsString(log_file)) {
                set_log_file(log_file->valuestring);
                log_message(LOG_LVL_DEBUG, "Set log file to %s", log_file->valuestring);
            }

            cJSON_Delete(config);
        } else {
            log_message(LOG_LVL_WARN, "Failed to parse config.json, using default settings");
        }
        free(config_data);
    } else {
        log_message(LOG_LVL_WARN, "Failed to read config.json, using default settings");
    }

    return 0;
}

void app_config_cleanup(void) {
    cleanup_logger();
}