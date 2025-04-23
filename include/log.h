#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#define BUFFER_SIZE 1024

enum {
    LOG_LVL_NONE = 0,
    LOG_LVL_ERROR,
    LOG_LVL_WARN,
    LOG_LVL_DEBUG
};

typedef struct {
    char log_file_path[256];
    unsigned char log_level;
} LoggerConfig;

extern LoggerConfig logger_config;

void init_logger(void);
void cleanup_logger(void);
void set_log_level(int level);
void set_log_file(const char *file_path);
void log_message(int level, const char *format, ...);

#endif