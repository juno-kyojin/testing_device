/**
 * @file parser_option.h
 * @brief Command line options parser and configuration manager
 *
 * This module parses command line arguments to configure system parameters
 * such as thread count, queue size, batch size, log level, and progress reporting.
 */

 #ifndef PARSER_OPTION_H
 #define PARSER_OPTION_H
 
 #include <stdbool.h>
 #include "../include/log.h"
 
 /**
  * @brief System configuration structure
  */
 typedef struct {
     int thread_count;           /**< Number of worker threads */
     int queue_size;             /**< Maximum task queue size */
     int batch_size;             /**< Size of batch operations */
     log_level_t log_level;      /**< Default logging level */
     int progress_interval;      /**< Interval between progress reports (in test cases) */
     char input_file[256];       /**< Path to input JSON file */
     char output_dir[256];       /**< Directory for output files */
     network_type_t network_type; /**< Network type to filter for (LAN/WAN) */
     bool compress_results;      /**< Whether to compress results before sending */
     char pc_host[128];          /**< PC App host address */
     int pc_port;                /**< PC App SSH port */
     char pc_username[64];       /**< PC App SSH username */
     char pc_key_path[256];      /**< Path to SSH private key */
     char pc_remote_dir[256];    /**< Remote directory on PC App */
 } system_config_t;
 
 /**
  * @brief Parse command line options
  *
  * @param argc Number of command line arguments
  * @param argv Array of command line argument strings
  * @param config Pointer to configuration structure to fill
  * @return int 0 on success, negative value on error
  */
 int parse_options(int argc, char* argv[], system_config_t* config);
 
 /**
  * @brief Parse configuration file
  *
  * @param config_file Path to configuration file
  * @param config Pointer to configuration structure to fill
  * @return int 0 on success, negative value on error
  */
 int parse_config_file(const char* config_file, system_config_t* config);
 
 /**
  * @brief Initialize configuration with default values
  *
  * @param config Pointer to configuration structure to initialize
  * @return int 0 on success, negative value on error
  */
 int init_default_config(system_config_t* config);
 
 /**
  * @brief Print help message
  *
  * @param program_name Name of the executable
  */
 void print_help(const char* program_name);
 
 /**
  * @brief Set the progress reporting interval
  *
  * @param interval Interval between progress reports (in test cases)
  * @return int 0 on success, negative value on error
  */
 int set_progress_interval(int interval);
 
 /**
  * @brief Apply system configuration
  *
  * @param config Pointer to configuration structure
  * @return int 0 on success, negative value on error
  */
 int apply_system_config(const system_config_t* config);
 
 #endif /* PARSER_OPTION_H */