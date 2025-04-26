#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#define BUFFER_SIZE 1024

/**
 * @brief Log levels for controlling verbosity of log messages
 */
enum {
    LOG_LVL_NONE = 0,   /**< No logging */
    LOG_LVL_ERROR,      /**< Only error messages */
    LOG_LVL_WARN,       /**< Error and warning messages */
    LOG_LVL_DEBUG       /**< All messages (errors, warnings, and debug info) */
};

/**
 * @brief Configuration structure for the logging system
 */
typedef struct {
    char log_file_path[256];  /**< Path to the log file */
    unsigned char log_level;  /**< Current logging level */
} LoggerConfig;

/**
 * @brief Global logger configuration
 */
extern LoggerConfig logger_config;

/**
 * @brief Initialize the logging system
 * 
 * Sets up the log file and prepares the logging system for use.
 */
void init_logger(void);

/**
 * @brief Clean up resources used by the logging system
 */
void cleanup_logger(void);

/**
 * @brief Set the logging verbosity level
 * 
 * @param level New log level (LOG_LVL_NONE, LOG_LVL_ERROR, LOG_LVL_WARN, or LOG_LVL_DEBUG)
 */
void set_log_level(int level);

/**
 * @brief Change the log file path
 * 
 * @param file_path Path to the new log file
 */
void set_log_file(const char *file_path);

/**
 * @brief Log a message with the specified level
 * 
 * If the current log_level is set to a value lower than the specified level,
 * the message will not be logged.
 * 
 * @param level Logging level for this message
 * @param format Printf-style format string
 * @param ... Variable arguments for the format string
 */
void log_message(int level, const char *format, ...);

#endif