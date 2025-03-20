
 #ifndef TC_H
 #define TC_H
 
 #include "parser_data.h"  // Để sử dụng cấu trúc test_case_t
 
 /**
  * @brief Trạng thái kết quả test
  */
 typedef enum {
     TEST_RESULT_SUCCESS,  /**< Test thành công */
     TEST_RESULT_FAILED,   /**< Test thất bại */
     TEST_RESULT_TIMEOUT,  /**< Test bị timeout */
     TEST_RESULT_ERROR     /**< Lỗi khi thực thi test */
 } test_result_status_t;
 
 /**
  * @brief Kết quả chi tiết cho ping test
  */
 typedef struct {
     int packets_sent;      /**< Số gói tin đã gửi */
     int packets_received;  /**< Số gói tin đã nhận */
     float min_rtt;         /**< RTT nhỏ nhất (ms) */
     float avg_rtt;         /**< RTT trung bình (ms) */
     float max_rtt;         /**< RTT lớn nhất (ms) */
     float packet_loss;     /**< Tỷ lệ mất gói (%) */
 } ping_result_t;
 
 /**
  * @brief Kết quả chi tiết cho throughput test
  */
 typedef struct {
     float bandwidth;       /**< Băng thông (Mbps) */
     int jitter;            /**< Jitter (ms) */
     int packet_loss;       /**< Mất gói (%) */
     float retransmits;     /**< Tỷ lệ gửi lại (%) */
 } throughput_result_t;
 
 /**
  * @brief Kết quả chi tiết cho security test
  */
 typedef struct {
     bool passed;           /**< Test bảo mật qua */
     int vulnerabilities;   /**< Số lỗ hổng tìm thấy */
     char vuln_details[256];/**< Chi tiết về lỗ hổng */
 } security_result_t;
 
 /**
  * @brief Cấu trúc kết quả test
  */
 typedef struct {
     char test_id[32];               /**< ID của test case */
     test_type_t test_type;          /**< Loại test */
     test_result_status_t status;    /**< Trạng thái kết quả */
     float execution_time;           /**< Thời gian thực thi (ms) */
     char result_details[1024];      /**< Chi tiết kết quả dạng text */
     
     /**
      * @brief Union chứa kết quả chi tiết tùy theo loại test
      */
     union {
         ping_result_t ping;             /**< Kết quả ping test */
         throughput_result_t throughput; /**< Kết quả throughput test */
         security_result_t security;     /**< Kết quả security test */
     } data;
 } test_result_info_t;
 
 /* Các hằng số và macro */
 /**
  * @brief Ngưỡng thời gian timeout mặc định (ms)
  */
 #define DEFAULT_TEST_TIMEOUT 10000  /* 10 giây */
 
 /**
  * @brief Ngưỡng tỷ lệ mất gói cho phép đối với ping test
  */
 #define PING_ACCEPTABLE_LOSS 20.0f  /* 20% */
 
 /**
  * @brief Ngưỡng RTT tối đa chấp nhận được cho ping test (ms)
  */
 #define PING_MAX_ACCEPTABLE_RTT 200.0f  /* 200ms */
 
 /* Khai báo các hàm */
 
 /**
  * @brief Thực thi test case
  * 
  * @param test_case Con trỏ đến test case
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int execute_test_case(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Thực thi ping test
  * 
  * @param test_case Con trỏ đến test case
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int execute_ping_test(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Thực thi throughput test
  * 
  * @param test_case Con trỏ đến test case
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int execute_throughput_test(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Tạo báo cáo tổng hợp từ các kết quả test
  * 
  * @param results Mảng kết quả test
  * @param count Số lượng kết quả
  * @param filename Đường dẫn đến file báo cáo
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int generate_summary_report(test_result_info_t *results, int count, const char *filename);
 
 /**
  * @brief Thực thi test case dựa trên loại mạng
  * 
  * @param test_case Con trỏ đến test case
  * @param network_type Loại mạng để thực thi (LAN hoặc WAN)
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int execute_test_case_by_network(test_case_t *test_case, network_type_t network_type, test_result_info_t *result);
 
 /**
  * @brief Chuyển đổi kết quả test sang định dạng JSON
  * 
  * @param result Con trỏ đến kết quả test
  * @param json_buffer Buffer để lưu chuỗi JSON
  * @param buffer_size Kích thước buffer
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int test_result_to_json(test_result_info_t *result, char *json_buffer, size_t buffer_size);
 
 /**
  * @brief Chuyển đổi trạng thái kết quả test sang chuỗi
  * 
  * @param status Trạng thái kết quả test
  * @return const char* Chuỗi mô tả trạng thái
  */
 const char* test_result_status_to_string(test_result_status_t status);
 
 #endif /* TC_H */