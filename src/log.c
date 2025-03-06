/**
 * @file log.c
 * @brief Implementation of custom logging system
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdarg.h>
 #include <time.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <sys/stat.h>
 #include <errno.h>
 
 #include "../include/log.h"
 #include "../include/file_process.h"
 
 #define LOG_BUFFER_SIZE 10000  // Number of log entries to buffer before flushing
 #define LOG_ENTRY_MAX_LEN 512  // Maximum length of a log entry
 
 // Log level names for printing
 static const char* log_level_names[] = {
     "NONE",
     "ERROR",
     "WARN",
     "INFO",
     "DEBUG"
 };
 
 // Log category names for printing
 static const char* log_category_names[] = {
     "GENERAL",
     "TEST_MANAGER",
     "TEST_PROCESSOR",
     "TEST_ENGINE",
     "RESULT_MANAGER",
     "FILE_PROCESS"
 };
 
 // Structure to hold a log entry in the buffer
 typedef struct {
     char message[LOG_ENTRY_MAX_LEN];
     int valid;  // 1 if entry is valid, 0 if empty
 } log_entry_t;
 
 // Logging state
 static FILE* log_file = NULL;
 static log_level_t log_levels[LOG_CAT_COUNT];
 static log_entry_t* log_buffer = NULL;
 static int log_buffer_index = 0;
 static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
 static pthread_t log_thread;
 static int log_thread_running = 0;
 static char log_filename[256] = {0};
 
 // Forward declarations for internal functions
 static void* log_thread_function(void* arg);
 static int ensure_log_directory_exists(const char* log_file);
 
 int log_init(const char* log_file, log_level_t default_level) {
     if (!log_file) {
         return -1;
     }
 
     pthread_mutex_lock(&log_mutex);
 
     // Clean up any existing resources
     if (log_file) {
         fclose(log_file);
         log_file = NULL;
     }
 
     if (log_buffer) {
         free(log_buffer);
         log_buffer = NULL;
     }
 
     // Ensure the log directory exists
     if (ensure_log_directory_exists(log_file) != 0) {
         pthread_mutex_unlock(&log_mutex);
         return -1;
     }
 
     // Save log filename for rotation purposes
     strncpy(log_filename, log_file, sizeof(log_filename) - 1);
     log_filename[sizeof(log_filename) - 1] = '\0';
 
     // Open log file
     log_file = open_file(log_file, FILE_MODE_APPEND);
     if (!log_file) {
         pthread_mutex_unlock(&log_mutex);
         return -1;
     }
 
     // Initialize log levels
     for (int i = 0; i < LOG_CAT_COUNT; i++) {
         log_levels[i] = default_level;
     }
 
     // Initialize log buffer
     log_buffer = (log_entry_t*)calloc(LOG_BUFFER_SIZE, sizeof(log_entry_t));
     if (!log_buffer) {
         fclose(log_file);
         log_file = NULL;
         pthread_mutex_unlock(&log_mutex);
         return -1;
     }
     log_buffer_index = 0;
 
     // Start the log thread for async logging
     log_thread_running = 1;
     if (pthread_create(&log_thread, NULL, log_thread_function, NULL) != 0) {
         free(log_buffer);
         log_buffer = NULL;
         fclose(log_file);
         log_file = NULL;
         log_thread_running = 0;
         pthread_mutex_unlock(&log_mutex);
         return -1;
     }
 
     pthread_mutex_unlock(&log_mutex);
 
     // Log the initialization
     LOG_INFO(LOG_CAT_GENERAL, "Logging system initialized with default level: %s", 
              log_level_names[default_level]);
 
     return 0;
 }
 
 void log_cleanup(void) {
     pthread_mutex_lock(&log_mutex);
 
     // Signal thread to stop and wait for it
     if (log_thread_running) {
         log_thread_running = 0;
         pthread_mutex_unlock(&log_mutex);
         pthread_join(log_thread, NULL);
         pthread_mutex_lock(&log_mutex);
     }
 
     // Flush any remaining log entries
     log_buffer_flush();
 
     // Clean up resources
     if (log_file) {
         fclose(log_file);
         log_file = NULL;
     }
 
     if (log_buffer) {
         free(log_buffer);
         log_buffer = NULL;
     }
 
     pthread_mutex_unlock(&log_mutex);
 }
 
 int log_set_level(log_category_t category, log_level_t level) {
     if (category >= LOG_CAT_COUNT || level > LOG_LVL_DEBUG) {
         return -1;
     }
 
     pthread_mutex_lock(&log_mutex);
     log_levels[category] = level;
     pthread_mutex_unlock(&log_mutex);
 
     LOG_INFO(LOG_CAT_GENERAL, "Log level for %s set to %s", 
              log_category_names[category], log_level_names[level]);
 
     return 0;
 }
 
 void log_set_all_levels(log_level_t level) {
     if (level > LOG_LVL_DEBUG) {
         return;
     }
 
     pthread_mutex_lock(&log_mutex);
     for (int i = 0; i < LOG_CAT_COUNT; i++) {
         log_levels[i] = level;
     }
     pthread_mutex_unlock(&log_mutex);
 
     LOG_INFO(LOG_CAT_GENERAL, "All log levels set to %s", log_level_names[level]);
 }
 
 log_level_t log_get_level(log_category_t category) {
     if (category >= LOG_CAT_COUNT) {
         return LOG_LVL_NONE;
     }
 
     pthread_mutex_lock(&log_mutex);
     log_level_t level = log_levels[category];
     pthread_mutex_unlock(&log_mutex);
 
     return level;
 }
 
 int custom_printf_log(log_level_t level, log_category_t category,
                      const char* file, int line, const char* function,
                      const char* format, ...) {
     if (!log_file || !format || category >= LOG_CAT_COUNT ||
         level > LOG_LVL_DEBUG || level == LOG_LVL_NONE) {
         return -1;
     }
 
     // Check if this log level should be logged for this category
     pthread_mutex_lock(&log_mutex);
     if (level > log_levels[category]) {
         pthread_mutex_unlock(&log_mutex);
         return 0;  // Silently ignore logs above the current level
     }
     pthread_mutex_unlock(&log_mutex);
 
     // Format the log message
     char message[LOG_ENTRY_MAX_LEN];
     va_list args;
     va_start(args, format);
     vsnprintf(message, sizeof(message), format, args);
     va_end(args);
 
     // Get current time
     time_t now = time(NULL);
     struct tm* timeinfo = localtime(&now);
     char timestamp[32];
     strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
 
     // Full log message with timestamp, level, category, location
     char full_message[LOG_ENTRY_MAX_LEN];
     snprintf(full_message, sizeof(full_message),
              "[%s] [%s] [%s] [%s:%d:%s] %s\n",
              timestamp,
              log_level_names[level],
              log_category_names[category],
              file ? strrchr(file, '/') ? strrchr(file, '/') + 1 : file : "?",
              line,
              function ? function : "?",
              message);
 
     // Write directly to the log file
     pthread_mutex_lock(&log_mutex);
     if (log_file) {
         fputs(full_message, log_file);
         fflush(log_file);
     }
     pthread_mutex_unlock(&log_mutex);
 
     return 0;
 }
 
 int log_async(log_level_t level, log_category_t category,
              const char* file, int line, const char* function,
              const char* format, ...) {
     if (!log_buffer || !format || category >= LOG_CAT_COUNT ||
         level > LOG_LVL_DEBUG || level == LOG_LVL_NONE) {
         return -1;
     }
 
     // Check if this log level should be logged for this category
     pthread_mutex_lock(&log_mutex);
     if (level > log_levels[category]) {
         pthread_mutex_unlock(&log_mutex);
         return 0;}