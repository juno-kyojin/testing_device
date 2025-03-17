/**
 * @file parser_option.h
 * @brief Định nghĩa các hàm để xử lý tham số dòng lệnh và file cấu hình
 */

 #ifndef PARSER_OPTION_H
 #define PARSER_OPTION_H
 
 #include <stdbool.h>
 #include "parser_data.h"
 
 /**
  * @brief Cấu trúc lưu trữ các tham số dòng lệnh và cấu hình
  */
 typedef struct {
     char config_file[256];         /**< Đường dẫn đến file cấu hình */
     char log_file[256];            /**< Đường dẫn đến file log */
     char output_directory[256];    /**< Thư mục lưu kết quả */
     network_type_t network_type;   /**< Loại mạng cần kiểm tra */
     int log_level;                 /**< Mức độ chi tiết của log */
     int thread_count;              /**< Số lượng thread xử lý song song */
     bool daemon_mode;              /**< Chạy ở chế độ daemon */
     bool verbose;                  /**< Hiển thị thông tin chi tiết */
     char connection_host[128];     /**< Địa chỉ host kết nối */
     int connection_port;           /**< Cổng kết nối */
     char username[64];             /**< Tên đăng nhập */
     char password[64];             /**< Mật khẩu */
 } cmd_options_t;
 
 /**
  * @brief Phân tích tham số dòng lệnh
  * 
  * @param argc Số lượng tham số
  * @param argv Mảng các tham số
  * @param options Con trỏ đến cấu trúc lưu tùy chọn
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int parse_command_line(int argc, char *argv[], cmd_options_t *options);
 
 /**
  * @brief Hiển thị hướng dẫn sử dụng
  * 
  * @param program_name Tên chương trình
  */
 void show_usage(const char *program_name);
 
 /**
  * @brief Đặt giá trị mặc định cho các tùy chọn
  * 
  * @param options Con trỏ đến cấu trúc lưu tùy chọn
  */
 void set_default_options(cmd_options_t *options);
 
 /**
  * @brief Đọc cấu hình từ file
  * 
  * @param config_file Đường dẫn đến file cấu hình
  * @param options Con trỏ đến cấu trúc lưu tùy chọn
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int read_config_file(const char *config_file, cmd_options_t *options);
 
 /**
  * @brief Kiểm tra tính hợp lệ của các tùy chọn
  * 
  * @param options Con trỏ đến cấu trúc lưu tùy chọn
  * @return int 0 nếu hợp lệ, -1 nếu không hợp lệ
  */
 int validate_options(const cmd_options_t *options);
 
 /**
  * @brief Hiển thị các tùy chọn hiện tại
  * 
  * @param options Con trỏ đến cấu trúc lưu tùy chọn
  */
 void print_options(const cmd_options_t *options);
 
 /**
  * @brief Chuyển đổi chuỗi thành loại mạng
  * 
  * @param network_str Chuỗi đại diện loại mạng
  * @return network_type_t Loại mạng tương ứng
  */
 network_type_t string_to_network_type(const char *network_str);
 
 /**
  * @brief Chuyển đổi loại mạng thành chuỗi
  * 
  * @param type Loại mạng
  * @return const char* Chuỗi đại diện loại mạng
  */
 const char* network_type_to_string(network_type_t type);
 
 #endif /* PARSER_OPTION_H */