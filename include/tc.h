#ifndef TC_H
#define TC_H

#include <stdbool.h>
#include "parser_data.h"  // Để sử dụng cấu trúc test_case_t

//---------- Các định nghĩa kiểu dữ liệu ----------//

/**
 * @brief Test result status
 */
typedef enum {
    TEST_RESULT_SUCCESS = 0,   /**< Test completed successfully */
    TEST_RESULT_FAILED = 1,    /**< Test failed */
    TEST_RESULT_TIMEOUT = 2,   /**< Test timed out */
    TEST_RESULT_ERROR = 3      /**< Test error */
} test_result_status_t;

/**
 * @brief Structure for ping test data
 */
typedef struct {
    int packets_sent;        /**< Number of packets sent */
    int packets_received;    /**< Number of packets received */
    float packet_loss;       /**< Packet loss percentage */
    float min_rtt;           /**< Minimum round trip time (ms) */
    float avg_rtt;           /**< Average round trip time (ms) */
    float max_rtt;           /**< Maximum round trip time (ms) */
} ping_data_t;

/**
 * @brief Structure for throughput test data
 */
typedef struct {
    float bandwidth;        /**< Bandwidth in Mbps */
    int jitter;             /**< Jitter in ms */
    int packet_loss;        /**< Packet loss percentage */
} throughput_data_t;

/**
 * @brief Structure for security test data
 */
typedef struct {
    bool passed;            /**< Whether test passed */
    int vulnerabilities;    /**< Number of vulnerabilities found */
} security_data_t;

/**
 * @brief Structure for speedtest data
 */
typedef struct {
    float download_speed;   /**< Download speed in Mbps */
    float upload_speed;     /**< Upload speed in Mbps */
    float latency;          /**< Latency in ms */
} speedtest_data_t;

/**
 * @brief Test result information
 */
typedef struct {
    char test_id[32];               /**< Test ID */
    test_result_status_t status;    /**< Test status */
    int test_type;                  /**< Test type */
    float execution_time;           /**< Test execution time (ms) */
    char result_details[1024];      /**< Detailed test results */
    
    union {
        ping_data_t ping;           /**< Ping test data */
        throughput_data_t throughput; /**< Throughput test data */
        security_data_t security;   /**< Security test data */
        speedtest_data_t speedtest; /**< Speedtest data */
    } data;                         /**< Test-specific data */
} test_result_info_t;

//---------- Các hàm hỗ trợ thời gian và timeout (trong tc.c) ----------//

/**
 * @brief Start timer for timeout
 */
void start_timer(void);

/**
 * @brief Stop timer and get elapsed time
 * 
 * @return float Elapsed time in ms
 */
float stop_timer(void);

/**
 * @brief Set timeout for operation
 * 
 * @param ms Timeout in milliseconds
 * @return int 0 on success, -1 on error
 */
int set_timeout(int ms);

/**
 * @brief Clear timeout
 */
void clear_timeout(void);

/**
 * @brief Check if timeout has occurred
 * 
 * @return bool true if timeout occurred, false otherwise
 */
bool is_timeout_occurred(void);

//---------- Các hàm xử lý kết quả test (trong tc.c) ----------//

/**
 * @brief Parse kết quả ping từ output
 * 
 * @param output Chuỗi output của lệnh ping
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int parse_ping_result(const char *output, ping_data_t *result);

/**
 * @brief Convert test result status to string
 * 
 * @param status The status to convert
 * @return const char* String representation of status
 */
const char* test_result_status_to_string(test_result_status_t status);

/**
 * @brief Tạo báo cáo tổng hợp từ các kết quả test
 * 
 * @param results Mảng kết quả test
 * @param count Số lượng kết quả
 * @param filename Đường dẫn đến file báo cáo
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int generate_summary_report(test_result_info_t *results, int count, const char *filename);

#endif /* TC_H */