/**
 * @file get_data.h
 * @brief Report management and data transfer functionality
 *
 * This module provides functions to create reports from custom logs
 * and send test results, reports, and progress back to PC App via SSH/SCP.
 */

 #ifndef GET_DATA_H
 #define GET_DATA_H
 
 #include <stdint.h>
 #include <stdbool.h>
 
 /**
  * @brief Configuration structure for SSH/SPC connection
  */
 typedef struct {
     char host[128];      /**< PC App host address */
     int port;            /**< SSH port (typically 22) */
     char username[64];   /**< SSH username */
     char password[128];  /**< SSH password or empty if using key auth */
     char key_path[256];  /**< Path to SSH private key if used */
     char remote_dir[256]; /**< Remote directory on PC App to send files to */
 } ssh_config_t;
 
 /**
  * @brief Initialize SSH connection configuration
  *
  * @param host PC App host address
  * @param port SSH port (typically 22)
  * @param username SSH username
  * @param password SSH password (can be NULL if using key auth)
  * @param key_path Path to SSH private key (can be NULL if using password auth)
  * @param remote_dir Remote directory on PC App to send files to
  * @return int 0 on success, negative value on error
  */
 int init_ssh_config(const char* host, int port, const char* username, 
                     const char* password, const char* key_path, 
                     const char* remote_dir);
 
 /**
  * @brief Generate test report with progress information
  *
  * @param log_file Path to the custom log file
  * @param report_file Path to save the generated report
  * @param total_cases Total number of test cases
  * @param batch_id Batch identifier
  * @param network_type Network type (e.g., "LAN", "WAN")
  * @return int 0 on success, negative value on error
  */
 int generate_test_report_with_progress(const char* log_file, const char* report_file,
                                       uint64_t total_cases, const char* batch_id,
                                       const char* network_type);
 
 /**
  * @brief Send compressed results to PC App via SCP
  *
  * @param results_file Path to the results file to send
  * @param report_file Path to the report file to send
  * @param compress Whether to compress files before sending
  * @return int 0 on success, negative value on error
  */
 int send_compressed_results_to_pc(const char* results_file, const char* report_file,
                                  bool compress);
 
 /**
  * @brief Send progress information to PC App via SSH
  *
  * @param processed Number of test cases processed
  * @param total Total number of test cases
  * @param batch_id Batch identifier
  * @return int 0 on success, negative value on error
  */
 int send_progress_to_pc(uint64_t processed, uint64_t total, const char* batch_id);
 
 /**
  * @brief Test SSH/SCP connection to PC App
  *
  * @return int 0 if connection successful, negative value otherwise
  */
 int test_ssh_connection(void);
 
 #endif /* GET_DATA_H */