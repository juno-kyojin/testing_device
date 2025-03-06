/**
 * @file log.h
 * @brief Custom logging system for the testing device
 *
 * Provides a custom logging system that supports various log levels,
 * asynchronous logging, and log rotation for optimal performance
 * with mass batch operations.
 */

 #ifndef LOG_H
 #define LOG_H
 
 #include <stdio.h>
 #include <stdbool.h>
 #include <stdint.h>
 
 /**
  * @brief Log levels for setting verbosity
  */
 typedef enum {
     LOG_LVL_NONE = 0,   /**< No logging */
     LOG_LVL_ERROR = 1,  /**< Error messages only */
     LOG_LVL_WARN = 2,   /**< Warnings and errors */
     LOG_LVL_INFO = 3,   /**< Informational messages */
     LOG_LVL_DEBUG = 4   /**< Debug messages (most verbose) */
 } log_level_t;
 
 /**
  * @brief Log categories for different components
  */
 typedef enum {
     LOG_CAT_GENERAL = 0,       /**< General logging */
     LOG_CAT_TEST_MANAGER,      /**< Test Case Manager logs */
     LOG_CAT_TEST_PROCESSOR,    /**< Test Case Processor logs */
     LOG_CAT_TEST_ENGINE,       /**< Test Execution Engine logs */
     LOG_CAT_RESULT_MANAGER,    /**< Result Manager logs */
     LOG_CAT_FILE_PROCESS,      /**< File Process logs */
     LOG_CAT_COUNT              /**< Number of log categories */
 } log_category_t;
 
 /**
  * @brief Initialize the logging system
  *
  * @param log_file Path to the log file
  * @param default_level Default log level to use
  * @return int 0 on success, negative value on error
  */
 int log_init(const char* log_file, log_level_t default_level);
 
 /**
  * @brief Clean up logging system resources
  */
 void log_cleanup(void);
 
 /**
  * @brief Set the log level for a specific category
  *
  * @param category Log category to set level for
  * @param level Desired log level
  * @return int 0 on success, negative value on error
  */
 int log_set_level(log_category_t category, log_level_t level);
 
 /**
  * @brief Set the log level for all categories
  *
  * @param level Desired log level
  */
 void log_set_all_levels(log_level_t level);
 
 /**
  * @brief Get current log level for a category
  *
  * @param category Log category to get level for
  * @return log_level_t Current log level
  */
 log_level_t log_get_level(log_category_t category);
 
 /**
  * @brief Write a log message synchronously
  *
  * @param level Log level of the message
  * @param category Log category of the message
  * @param file Source file name
  * @param line Source line number
  * @param function Function name
  * @param format Format string for the message
  * @return int 0 on success, negative value on error
  */
 int custom_printf_log(log_level_t level, log_category_t category, 
                      const char* file, int line, const char* function, 
                      const char* format, ...);
 
 /**
  * @brief Write a log message asynchronously
  *
  * @param level Log level of the message
  * @param category Log category of the message
  * @param file Source file name
  * @param line Source line number
  * @param function Function name
  * @param format Format string for the message
  * @return int 0 on success, negative value on error
  */
 int log_async(log_level_t level, log_category_t category, 
              const char* file, int line, const char* function, 
              const char* format, ...);
 
 /**
  * @brief Flush the log buffer to disk
  *
  * @return int Number of log entries flushed or negative value on error
  */
 int log_buffer_flush(void);
 
 /**
  * @brief Log progress of batch processing
  *
  * @param processed Number of test cases processed
  * @param total Total number of test cases
  * @return int 0 on success, negative value on error
  */
 int log_progress(uint64_t processed, uint64_t total);
 
 /**
  * @brief Rotate log file if it exceeds size limit
  *
  * @param max_size Maximum log file size in bytes before rotation
  * @return int 0 on success, negative value on error
  */
 int rotate_log_file(size_t max_size);
 
 // Convenience macros for logging
 #define LOG_ERROR(cat, fmt, ...) custom_printf_log(LOG_LVL_ERROR, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 #define LOG_WARN(cat, fmt, ...)  custom_printf_log(LOG_LVL_WARN, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 #define LOG_INFO(cat, fmt, ...)  custom_printf_log(LOG_LVL_INFO, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 #define LOG_DEBUG(cat, fmt, ...) custom_printf_log(LOG_LVL_DEBUG, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 
 // Asynchronous logging macros
 #define ALOG_ERROR(cat, fmt, ...) log_async(LOG_LVL_ERROR, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 #define ALOG_WARN(cat, fmt, ...)  log_async(LOG_LVL_WARN, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 #define ALOG_INFO(cat, fmt, ...)  log_async(LOG_LVL_INFO, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 #define ALOG_DEBUG(cat, fmt, ...) log_async(LOG_LVL_DEBUG, cat, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
 
 #endif /* LOG_H */