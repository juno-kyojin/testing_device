/**
 * @file get_data.h
 * @brief Định nghĩa các hàm giao tiếp giữa thiết bị và PC qua SSH
 */

 #ifndef GET_DATA_H
 #define GET_DATA_H
 
 #include <stdbool.h>
 #include <libssh/libssh.h>
 #include "tc.h"  // Để truy cập định nghĩa của test_result_info_t
 #include <libssh/server.h>  // Defines ssh_bind type
 
 // Các mã yêu cầu từ PC
 #define REQUEST_DISCONNECT      0
 #define REQUEST_REPORT          1
 #define REQUEST_DETAILED_REPORT 2
 #define REQUEST_RAW_DATA        3
 #define REQUEST_LOGS            4
 
 // Các loại báo cáo
 #define REPORT_TYPE_SUMMARY     1
 #define REPORT_TYPE_DETAILED    2
 #define REPORT_TYPE_RAW         3
 
 /**
  * @brief Cấu trúc lưu thông tin kết nối SSH
  */
 typedef struct {
     char host[256];        /**< Địa chỉ host PC */
     int port;              /**< Cổng SSH */
     char username[64];     /**< Tên đăng nhập */
     char password[64];     /**< Mật khẩu */
     ssh_session session;   /**< Session SSH */
     char client_ip[64];    /**< IP của client kết nối tới (khi device là server) */
     bool auto_send_report; /**< Tự động gửi báo cáo khi test hoàn tất */
 } connection_info_t;
 
 // [Các khai báo hàm khác giữ nguyên]
 
 /**
  * @brief Chờ và xử lý yêu cầu từ PC
  * 
  * @param conn Con trỏ đến thông tin kết nối
  * @param timeout_ms Thời gian timeout cho việc chờ (ms)
  * @return int Mã yêu cầu nhận được, hoặc -1 nếu timeout/lỗi
  */
 int wait_for_pc_request(connection_info_t* conn, int timeout_ms);
 
 /**
  * @brief Gửi báo cáo theo yêu cầu của PC
  * 
  * @param conn Con trỏ đến thông tin kết nối
  * @param report_file Đường dẫn đến file báo cáo
  * @param report_type Loại báo cáo (summary, detail, raw)
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int send_report_on_request(connection_info_t* conn, const char* report_file, int report_type);
 
 /**
  * @brief Thiết lập device làm SSH server để nhận kết nối
  * 
  * @param port Cổng SSH server
  * @param host_key_file Đường dẫn đến host key
  * @return ssh_bind Con trỏ đến bind server hoặc NULL nếu thất bại
  */
 ssh_bind start_ssh_server(int port, const char* host_key_file);
 
 /**
  * @brief Chấp nhận kết nối SSH từ PC
  * 
  * @param sshbind Con trỏ đến ssh_bind server
  * @param info Con trỏ đến biến lưu thông tin kết nối
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int accept_connection(ssh_bind sshbind, connection_info_t* info);
 
 #endif // GET_DATA_H