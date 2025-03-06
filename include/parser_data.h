/**
 * @file parser_data.h
 * @brief Test case management and JSON parsing
 *
 * This module defines the test case structure and provides functions
 * for reading, filtering, and managing test cases from JSON files.
 */

 #ifndef PARSER_DATA_H
 #define PARSER_DATA_H
 
 #include <stdint.h>
 #include <stdbool.h>
 
 /**
  * @brief Network types supported by the test framework
  */
 typedef enum {
     NETWORK_TYPE_LAN,     /**< Local Area Network */
     NETWORK_TYPE_WAN,     /**< Wide Area Network */
     NETWORK_TYPE_UNKNOWN  /**< Unknown network type */
 } network_type_t;
 
 /**
  * @brief Protocol types for test cases
  */
 typedef enum {
     PROTOCOL_TCP,    /**< TCP protocol */
     PROTOCOL_UDP,    /**< UDP protocol */
     PROTOCOL_ICMP,   /**< ICMP protocol */
     PROTOCOL_HTTP,   /**< HTTP protocol */
     PROTOCOL_HTTPS   /**< HTTPS protocol */
 } protocol_t;
 
 /**
  * @brief Test case structure
  */
 typedef struct {
     uint64_t id;                 /**< Unique test case identifier */
     char target[256];            /**< Target address/hostname */
     protocol_t protocol;         /**< Protocol to use */
     uint16_t port;               /**< Port number */
     network_type_t network_type; /**< Network type (LAN/WAN) */
     bool valid;                  /**< Flag indicating if test case is valid */
 } test_case_t;
 
 /**
  * @brief Read test cases from a JSON file
  *
  * @param filename JSON file path to read from
  * @param test_cases Array to store test cases
  * @param max_cases Maximum number of test cases to read
  * @param decompress Whether to decompress the file first
  * @return int Number of test cases read or negative value on error
  */
 int read_json_test_cases(const char* filename, test_case_t* test_cases, 
                         int max_cases, bool decompress);
 
 /**
  * @brief Filter valid test cases based on network type
  *
  * @param test_cases Array of test cases to filter
  * @param num_cases Number of test cases in array
  * @param network_type Network type to filter for
  * @return int Number of valid test cases after filtering
  */
 int filter_valid_test_cases(test_case_t* test_cases, int num_cases, 
                            network_type_t network_type);
 
 /**
  * @brief Enqueue test cases to the processing queue
  *
  * @param test_cases Array of test cases to enqueue
  * @param num_cases Number of test cases in array
  * @return int Number of successfully enqueued test cases or negative value on error
  */
 int enqueue_test_cases(const test_case_t* test_cases, int num_cases);
 
 /**
  * @brief Get test case progress
  *
  * @param processed Number of test cases processed
  * @param total Total number of test cases
  * @return double Progress percentage (0-100)
  */
 double get_test_case_progress(uint64_t processed, uint64_t total);
 
 /**
  * @brief Log test case progress
  *
  * @param processed Number of test cases processed
  * @param total Total number of test cases
  * @param batch_id Batch identifier
  * @return int 0 on success, negative value on error
  */
 int log_test_case_progress(uint64_t processed, uint64_t total, const char* batch_id);
 
 /**
  * @brief Convert string to network type
  *
  * @param str Network type string ("LAN", "WAN", etc.)
  * @return network_type_t Corresponding network type or NETWORK_TYPE_UNKNOWN
  */
 network_type_t string_to_network_type(const char* str);
 
 /**
  * @brief Convert string to protocol type
  *
  * @param str Protocol string ("TCP", "UDP", "ICMP", etc.)
  * @return protocol_t Corresponding protocol type
  */
 protocol_t string_to_protocol(const char* str);
 
 #endif /* PARSER_DATA_H */