#ifndef TC_H
 #define TC_H
 
 #include "parser_data.h"  // Để sử dụng cấu trúc test_case_t
 
 //---------- Các định nghĩa kiểu dữ liệu ----------//
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
  * @brief Kết quả chi tiết cho speedtest
  */
 typedef struct {
     float download_speed;  /**< Tốc độ tải xuống (Mbps) */
     float upload_speed;    /**< Tốc độ tải lên (Mbps) */
     float latency;         /**< Độ trễ (ms) */
 } speedtest_result_t;
 
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
         speedtest_result_t speedtest;   /**< Kết quả speedtest */
     } data;
 } test_result_info_t;
 
 //---------- Các hàm hỗ trợ thời gian và timeout (trong tc.c) ----------//
 
 /**
  * @brief Bắt đầu đếm thời gian
  */
 void start_timer(void);
 
 /**
  * @brief Dừng đếm thời gian và trả về thời gian đã trôi qua (ms)
  * @return Thời gian đã trôi qua tính bằng ms
  */
 float stop_timer(void);
 
 /**
  * @brief Thiết lập timeout cho test
  * 
  * @param timeout_ms Thời gian timeout tính bằng ms
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int set_timeout(int timeout_ms);
 
 /**
  * @brief Hủy timeout
  */
 void clear_timeout(void);
 
 /**
  * @brief Kiểm tra xem timeout đã xảy ra chưa
  * 
  * @return int 1 nếu timeout đã xảy ra, 0 nếu chưa
  */
 int is_timeout_occurred(void);
 
 //---------- Các hàm xử lý kết quả test (trong tc.c) ----------//
 
 /**
  * @brief Parse kết quả ping từ output
  * 
  * @param output Chuỗi output của lệnh ping
  * @param result Con trỏ đến biến lưu kết quả
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int parse_ping_result(const char *output, ping_result_t *result);
 
 /**
  * @brief Chuyển đổi trạng thái kết quả test sang chuỗi
  * 
  * @param status Trạng thái kết quả test
  * @return const char* Chuỗi mô tả trạng thái
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