#include "log.h"
#include "file_process.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>

const char *log_level_strings[] = {"NONE", "ERROR", "WARN", "DEBUG"};
LoggerConfig logger_config = {
    .log_file_path = "logs/default.log",
    .log_level = LOG_LVL_DEBUG
};

void init_logger(void) {
    // Tạo hoặc mở file log với chế độ append để giữ log cũ
    FILE *fp = open_file(logger_config.log_file_path, FILE_MODE_APPEND, 3);
    if (fp) {
        fclose(fp);
    }
    log_message(LOG_LVL_DEBUG, "Logger initialized");
}

void cleanup_logger(void) {
    log_message(LOG_LVL_DEBUG, "Logger cleaned up");
}

void set_log_level(int level) {
    logger_config.log_level = (unsigned char)level;
}

void set_log_file(const char *file_path) {
    strncpy(logger_config.log_file_path, file_path, sizeof(logger_config.log_file_path) - 1);
    logger_config.log_file_path[sizeof(logger_config.log_file_path) - 1] = '\0';
    FILE *fp = open_file(logger_config.log_file_path, FILE_MODE_APPEND, 3);
    if (fp) {
        fclose(fp);
    }
    log_message(LOG_LVL_DEBUG, "Set log file to: %s", logger_config.log_file_path);
}

void log_message(int level, const char *format, ...) {
    if (level <= logger_config.log_level) {
        char buffer[BUFFER_SIZE];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, BUFFER_SIZE, format, args);
        va_end(args);

        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);
        char full_log[BUFFER_SIZE];
        snprintf(full_log, BUFFER_SIZE, "[%s][%s] %s", timestamp, log_level_strings[level], buffer);

        // Ghi log trực tiếp theo Log Flow
        FILE *fp = open_file(logger_config.log_file_path, FILE_MODE_APPEND, 3);
        if (fp) {
            fprintf(fp, "%s\n", full_log);
            fflush(fp);
            fclose(fp);
        }
    }
}