/**
 * @file tc.h
 * @brief Định nghĩa các hàm thực thi test case và xử lý kết quả
 */

#ifndef TC_H
#define TC_H

#include "parser_data.h"

// Result status
typedef enum {
    TEST_RESULT_SUCCESS,
    TEST_RESULT_FAILED,
    TEST_RESULT_TIMEOUT,
    TEST_RESULT_ERROR
} test_result_status_t;

// Test result structure
typedef struct {
    char test_id[32];
    int test_type;
    test_result_status_t status;
    float execution_time;
    char result_details[1024];
    
    union {
        struct {
            int packets_sent;
            int packets_received;
            float min_rtt;
            float avg_rtt;
            float max_rtt;
        } ping;
        
        struct {
            float bandwidth;
            int jitter;
            int packet_loss;
        } throughput;
    } data;
} test_result_info_t;

// Function declarations
int execute_test_case(test_case_t *test_case, test_result_info_t *result);
int execute_ping_test(test_case_t *test_case, test_result_info_t *result);
int execute_throughput_test(test_case_t *test_case, test_result_info_t *result);
int generate_summary_report(test_result_info_t *results, int count, const char *filename);

#endif // TC_H