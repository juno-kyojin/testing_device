/**
 * @file tc.h
 * @brief Định nghĩa các hàm thực thi test case và xử lý kết quả
 */

 #ifndef TC_H
 #define TC_H
 
 #include "parser_data.h"
 
 /**
  * @brief Trạng thái thực thi test case
  */
 typedef enum {
     TEST_RESULT_SUCCESS,       /**< Test case thành công */
     TEST_RESULT_FAILED,        /**< Test case thất bại */
     TEST_RESULT_TIMEOUT,       /**< Test case bị timeout */
     TEST_RESULT_ERROR          /**< Lỗi khi thực thi */
 } test_result_status_t;
 
 /**
  * @brief Cấu trúc lưu thông tin kết quả của một test case
  */
 typedef struct {
     char test_id[32];              /**< ID của test case */
     test_type_t test_type;         /**< Loại test case */
     test_result_status_t status;   /**< Trạng thái thực thi */
     float execution_time;          /**< Thời gian thực thi (ms) */
     char result_details[1024];     /**< Chi tiết kết quả */
     
     /* Kết quả chi tiết tùy theo loại test */
     union {
         struct {
             int packets_sent;      /**< Số gói tin đã gửi */
             int packets_received;  /**< Số gói tin đã nhận */
             float min_rtt;         /**< RTT tối thiểu (ms) */
             float avg_rtt;         /**< RTT trung bình (ms) */
             float max_rtt;         /**< RTT tối đa (ms) */
         } ping;
         
         struct {
             float bandwidth;       /**< Băng thông đo được (Mbps) */
             int jitter;            /**< Jitter (ms) */
             int packet_loss;       /**< Tỷ lệ mất gói (%) */
         } throughput;
         
         struct {
             bool vlan_detected;    /**< VLAN có được phát hiện không */
             int frames_sent;       /**< Số frame đã gửi */
             int frames_received;   /**< Số frame đã nhận */
         } vlan;
         
         struct {
             int vulnerabilities;   /**< Số lỗ hổng phát hiện */
             char details[512];     /**< Chi tiết lỗ hổng */
         } security;
     } data;
 } test_result_info_t;
 
 /**
  * @brief Thực thi một test case
  * 
  * @param test_case Con trỏ đến test case cần thực thi
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thực thi thành công, -1 nếu có lỗi
  */
 int execute_test_case(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Thực thi test case dựa vào loại mạng
  * 
  * @param test_case Con trỏ đến test case
  * @param network_type Loại mạng cần thực thi
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thực thi thành công, -1 nếu có lỗi
  */
 int execute_test_case_by_network(test_case_t *test_case, network_type_t network_type, test_result_info_t *result);
 
 /**
  * @brief Thực thi test ping
  * 
  * @param test_case Con trỏ đến test case kiểu ping
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thực thi thành công, -1 nếu có lỗi
  */
 int execute_ping_test(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Thực thi test throughput
  * 
  * @param test_case Con trỏ đến test case kiểu throughput
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thực thi thành công, -1 nếu có lỗi
  */
 int execute_throughput_test(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Thực thi test VLAN
  * 
  * @param test_case Con trỏ đến test case kiểu VLAN
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thực thi thành công, -1 nếu có lỗi
  */
 int execute_vlan_test(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Thực thi test bảo mật
  * 
  * @param test_case Con trỏ đến test case kiểu bảo mật
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thực thi thành công, -1 nếu có lỗi
  */
 int execute_security_test(test_case_t *test_case, test_result_info_t *result);
 
 /**
  * @brief Lưu kết quả test case vào file
  * 
  * @param result Con trỏ đến kết quả
  * @param output_file Đường dẫn tới file đầu ra
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int save_test_result(test_result_info_t *result, const char *output_file);
 
 /**
  * @brief Tạo báo cáo tổng hợp từ nhiều kết quả test case
  * 
  * @param results Mảng các kết quả test case
  * @param count Số lượng kết quả
  * @param output_file Đường dẫn tới file báo cáo
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int generate_summary_report(test_result_info_t *results, int count, const char *output_file);
 
 #endif /* TC_H */