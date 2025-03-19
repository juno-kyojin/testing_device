/**
 * @file parser_data.h
 * @brief Định nghĩa cấu trúc test case và hàm xử lý
 */

 #ifndef PARSER_DATA_H
 #define PARSER_DATA_H
 
 #include <stdbool.h>
 #include <stddef.h>
 
 /**
  * @brief Loại mạng cho test case
  */
 typedef enum {
     NETWORK_LAN,   /**< Mạng LAN */
     NETWORK_WAN,   /**< Mạng WAN */
     NETWORK_BOTH   /**< Cả hai loại mạng */
 } network_type_t;
 
 /**
  * @brief Loại test case
  */
 typedef enum {
     TEST_PING,             /**< Kiểm tra ping */
     TEST_THROUGHPUT,       /**< Kiểm tra throughput */
     TEST_SECURITY,         /**< Kiểm tra bảo mật */
     TEST_OTHER             /**< Các loại kiểm tra khác */
 } test_type_t;
 
 /**
  * @brief Cấu trúc chung cho các tham số ping test
  */
 typedef struct {
     int count;          /**< Số lượng ping */
     int size;           /**< Kích thước gói tin */
     int interval;       /**< Khoảng thời gian giữa các ping (ms) */
     bool ipv6;          /**< Sử dụng IPv6 */
 } ping_params_t;
 
 /**
  * @brief Cấu trúc chung cho các tham số throughput test
  */
 typedef struct {
     int duration;       /**< Thời gian đo (giây) */
     char protocol[8];   /**< TCP/UDP */
     int port;           /**< Cổng */
     int buffer_size;    /**< Kích thước buffer */
     bool bidirectional; /**< Test hai chiều */
 } throughput_params_t;
 
 /**
  * @brief Cấu trúc chung cho các tham số security test
  */
 typedef struct {
     char method[32];    /**< Phương thức kiểm tra bảo mật */
     int port;           /**< Cổng */
     bool tls;           /**< Sử dụng TLS */
 } security_params_t;
 
 /**
  * @brief Cấu trúc lưu thông tin của một test case
  */
 typedef struct {
     char id[32];                /**< ID của test case */
     test_type_t type;           /**< Loại test case */
     network_type_t network_type;/**< Loại mạng */
     char name[64];              /**< Tên test case */
     char description[256];      /**< Mô tả test case */
     char target[128];           /**< Đích thực thi test case */
     int timeout;                /**< Thời gian timeout (ms) */
     bool enabled;               /**< Test case có được bật hay không */
     
     /* Tham số tùy theo loại test */
     union {
         ping_params_t ping;         /**< Tham số cho ping test */
         throughput_params_t throughput; /**< Tham số cho throughput test */
         security_params_t security;  /**< Tham số cho security test */
     } params;
     
     /* Dữ liệu bổ sung nếu cần */
     void *extra_data;           /**< Con trỏ đến dữ liệu bổ sung */
     size_t extra_data_size;     /**< Kích thước dữ liệu bổ sung */
 } test_case_t;
 
 /**
  * @brief Đọc test cases từ file JSON
  * 
  * @param json_file Đường dẫn đến file JSON
  * @param test_cases Con trỏ đến mảng test cases
  * @param count Con trỏ đến biến lưu số lượng test cases đọc được
  * @return true nếu thành công, false nếu thất bại
  */
 bool read_json_test_cases(const char *json_file, test_case_t **test_cases, int *count);
 
 /**
  * @brief Phân tích nội dung JSON trực tiếp thành test cases
  * 
  * @param json_content Chuỗi JSON chứa test cases
  * @param test_cases Con trỏ đến mảng test cases
  * @param count Con trỏ đến biến lưu số lượng test cases
  * @return true nếu thành công, false nếu thất bại
  */
 bool parse_json_content(const char *json_content, test_case_t **test_cases, int *count);
 
 /**
  * @brief Lọc test cases dựa trên loại mạng
  * 
  * @param test_cases Mảng test cases đầu vào
  * @param count Số lượng test cases đầu vào
  * @param network_type Loại mạng cần lọc
  * @param filtered_test_cases Con trỏ đến mảng test cases sau khi lọc
  * @param filtered_count Con trỏ đến biến lưu số lượng test cases sau khi lọc
  * @return true nếu thành công, false nếu thất bại
  */
 bool filter_test_cases_by_network(const test_case_t *test_cases, int count, 
                                 network_type_t network_type, 
                                 test_case_t **filtered_test_cases, 
                                 int *filtered_count);
 
 /**
  * @brief Giải phóng bộ nhớ của mảng test cases
  * 
  * @param test_cases Mảng test cases cần giải phóng
  * @param count Số lượng test cases
  */
 void free_test_cases(test_case_t *test_cases, int count);
 
 /**
  * @brief Chuyển đổi một test case thành chuỗi JSON
  * 
  * @param test_case Test case cần chuyển đổi
  * @param json_buffer Buffer chứa chuỗi JSON kết quả
  * @param buffer_size Kích thước buffer
  * @return true nếu thành công, false nếu thất bại
  */
 bool test_case_to_json(const test_case_t *test_case, char *json_buffer, size_t buffer_size);
 
 /**
  * @brief Chuyển đổi mảng test cases thành chuỗi JSON
  * 
  * @param test_cases Mảng test cases cần chuyển đổi
  * @param count Số lượng test cases
  * @param json_buffer Buffer chứa chuỗi JSON kết quả
  * @param buffer_size Kích thước buffer
  * @return true nếu thành công, false nếu thất bại
  */
 bool test_cases_to_json(const test_case_t *test_cases, int count, char *json_buffer, size_t buffer_size);
 
 #endif /* PARSER_DATA_H */