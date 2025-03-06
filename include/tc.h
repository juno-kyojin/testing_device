/**
 * @file tc.h
 * @brief Test case execution functionality
 *
 * This module provides functions for executing test cases on ONT devices,
 * supporting different network types (LAN, WAN), and reporting execution
 * progress.
 */

 #ifndef TC_H
 #define TC_H
 
 #include <stdint.h>
 #include "../include/parser_data.h"
 
 /**
  * @brief Test case execution results
  */
 typedef enum {
     TC_RESULT_SUCCESS = 0,     /**< Test case executed successfully */
     TC_RESULT_NETWORK_ERROR,   /**< Network error occurred */
     TC_RESULT_TIMEOUT,         /**< Execution timed out */
     TC_RESULT_INVALID_PARAM,   /**< Invalid parameter */
     TC_RESULT_PROTOCOL_ERROR,  /**< Protocol-specific error */
     TC_RESULT_UNKNOWN_ERROR    /**< Unknown error occurred */
 } tc_result_t;
 
 /**
  * @brief Execute a test case
  *
  * @param test_case Test case to execute
  * @return int Result code (0 for success, non-zero for failure)
  */
 int execute_test_case(const test_case_t* test_case);
 
 /**
  * @brief Execute a test case based on network type
  *
  * @param test_case Test case to execute
  * @return int Result code (0 for success, non-zero for failure)
  */
 int execute_test_case_by_network(const test_case_t* test_case);
 
 /**
  * @brief Report execution progress
  *
  * @param processed Number of test cases processed
  * @param total Total number of test cases
  * @param batch_id Batch identifier
  * @return int 0 on success, negative value on error
  */
 int report_execution_progress(uint64_t processed, uint64_t total, const char* batch_id);
 
 /**
  * @brief Log execution progress
  *
  * @param processed Number of test cases processed
  * @param total Total number of test cases
  * @param batch_id Batch identifier
  * @return int 0 on success, negative value on error
  */
 int log_execution_progress(uint64_t processed, uint64_t total, const char* batch_id);
 
 /**
  * @brief Execute a LAN network test
  *
  * @param test_case Test case to execute
  * @return int Result code (0 for success, non-zero for failure)
  */
 int execute_lan_test(const test_case_t* test_case);
 
 /**
  * @brief Execute a WAN network test
  *
  * @param test_case Test case to execute
  * @return int Result code (0 for success, non-zero for failure)
  */
 int execute_wan_test(const test_case_t* test_case);
 
 /**
  * @brief Initialize the test execution engine
  *
  * @return int 0 on success, negative value on error
  */
 int init_test_execution_engine(void);
 
 /**
  * @brief Clean up test execution engine resources
  *
  * @return int 0 on success, negative value on error
  */
 int cleanup_test_execution_engine(void);
 
 #endif /* TC_H */