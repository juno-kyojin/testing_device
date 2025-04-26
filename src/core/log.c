/**
 * @file log.c
 * @brief Implementation of the logging system
 * 
 * Provides functions for logging messages with different severity levels
 * to both file and standard error output.
 */
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

/**
 * @brief Maximum size of a single log line
 */
#define MAX_LOG_LINE_SIZE 2048

/**
 * @brief Default logger configuration
 */
LoggerConfig logger_config = {
    .log_file_path = "application.log",
    .log_level = LOG_LVL_DEBUG
};

/**
 * @brief Mutex for thread-safe logging
 */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief String representations of log levels
 */
static const char *log_level_names[] = {
    "NONE",
    "ERROR",
    "WARN",
    "DEBUG"
};

void init_logger(void) {
    FILE *log_file = fopen(logger_config.log_file_path, "a");
    if (log_file) {
        fclose(log_file);
    } else {
        fprintf(stderr, "Cannot open log file %s. Using stderr for logging.\n", logger_config.log_file_path);
    }
}

void cleanup_logger(void) {
}

void set_log_level(int level) {
    if (level >= LOG_LVL_NONE && level <= LOG_LVL_DEBUG) {
        logger_config.log_level = level;
    }
}

void set_log_file(const char *file_path) {
    if (file_path) {
        strncpy(logger_config.log_file_path, file_path, sizeof(logger_config.log_file_path) - 1);
        logger_config.log_file_path[sizeof(logger_config.log_file_path) - 1] = '\0';
        
        FILE *log_file = fopen(logger_config.log_file_path, "a");
        if (log_file) {
            fclose(log_file);
        } else {
            fprintf(stderr, "Cannot open new log file %s. Using previous log file.\n", file_path);
        }
    }
}

void log_message(int level, const char *format, ...) {
    if (level <= LOG_LVL_NONE || level > LOG_LVL_DEBUG || level > logger_config.log_level) {
        return;
    }
    
 // get the current time
 time_t now = time(NULL);
 struct tm tm_now;
 localtime_r(&now, &tm_now);
 
 char time_str[20];
 strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_now);
 
 // log message buffer
 char log_buffer[MAX_LOG_LINE_SIZE];
 
 // Header with timestamp and log level
 int header_len = snprintf(log_buffer, sizeof(log_buffer), "[%s] %s: ", 
                          time_str, log_level_names[level]);
 
 if (header_len < 0 || (size_t)header_len >= sizeof(log_buffer)) {
     return; 
 }
 
 // log content
 va_list args;
 va_start(args, format);
 int content_len = vsnprintf(log_buffer + header_len, sizeof(log_buffer) - header_len, 
                           format, args);
 va_end(args);
 
 if (content_len < 0) {
     return; // 
 }
 
// Ensure there is a newline character at the end
 size_t total_len = header_len + content_len;
 if (total_len < sizeof(log_buffer) - 2) {
     if (log_buffer[total_len - 1] != '\n') {
         log_buffer[total_len] = '\n';
         log_buffer[total_len + 1] = '\0';
     }
 } else {
   // Add a newline character for overly long strings
     log_buffer[sizeof(log_buffer) - 2] = '\n';
     log_buffer[sizeof(log_buffer) - 1] = '\0';
 }
 
 pthread_mutex_lock(&log_mutex);
 
// Write log to file
 FILE *log_file = fopen(logger_config.log_file_path, "a");
 if (log_file) {
     fputs(log_buffer, log_file);
     fclose(log_file);
 } else {
     fputs(log_buffer, stderr);
 }
 
 pthread_mutex_unlock(&log_mutex);
}