
 #ifndef FILE_PROCESS_H
 #define FILE_PROCESS_H
 
 #include <stdlib.h>
 #include <sys/types.h>
 #include <time.h>
 
 /**
  * @brief Đọc toàn bộ nội dung của file vào buffer
  * 
  * @param file_path Đường dẫn đến file cần đọc
  * @param buffer Con trỏ đến con trỏ buffer sẽ được cấp phát
  * @param size Con trỏ đến biến lưu kích thước dữ liệu đọc được
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int read_file(const char *file_path, char **buffer, size_t *size);
 
 /**
  * @brief Ghi dữ liệu vào file
  * 
  * @param file_path Đường dẫn đến file cần ghi
  * @param buffer Con trỏ đến buffer chứa dữ liệu
  * @param size Kích thước dữ liệu cần ghi
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int write_file(const char *file_path, const char *buffer, size_t size);
 
 /**
  * @brief Thêm dữ liệu vào cuối file
  * 
  * @param file_path Đường dẫn đến file
  * @param buffer Con trỏ đến buffer chứa dữ liệu
  * @param size Kích thước dữ liệu cần thêm
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int append_to_file(const char *file_path, const char *buffer, size_t size);
 
 /**
  * @brief Kiểm tra file có tồn tại không
  * 
  * @param file_path Đường dẫn đến file
  * @return int 1 nếu tồn tại, 0 nếu không tồn tại
  */
 int file_exists(const char *file_path);
 
 /**
  * @brief Tạo thư mục mới
  * 
  * @param dir_path Đường dẫn thư mục cần tạo
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int create_directory(const char *dir_path);
 
 /**
  * @brief Xóa file
  * 
  * @param file_path Đường dẫn đến file cần xóa
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int delete_file(const char *file_path);
 
 /**
  * @brief Sao chép file
  * 
  * @param src_path Đường dẫn đến file nguồn
  * @param dest_path Đường dẫn đến file đích
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int copy_file(const char *src_path, const char *dest_path);
 
 /**
  * @brief Lấy kích thước của file
  * 
  * @param file_path Đường dẫn đến file
  * @return long Kích thước của file, -1 nếu thất bại
  */
 long get_file_size(const char *file_path);
 
 /**
  * @brief Lấy thời gian sửa đổi cuối của file
  * 
  * @param file_path Đường dẫn đến file
  * @return time_t Thời gian sửa đổi, -1 nếu thất bại
  */
 time_t get_file_modification_time(const char *file_path);
 
 /**
  * @brief Tạo file tạm thời
  * 
  * @param prefix Tiền tố cho tên file
  * @param temp_path Buffer để lưu đường dẫn đến file tạm
  * @param path_size Kích thước của buffer temp_path
  * @return char* Con trỏ đến temp_path nếu thành công, NULL nếu thất bại
  */
 char* create_temp_file(const char *prefix, char *temp_path, size_t path_size);
 
 /**
  * @brief Đọc một phần của file từ vị trí offset
  * 
  * @param file_path Đường dẫn đến file
  * @param buffer Buffer để lưu dữ liệu đọc được
  * @param buffer_size Kích thước của buffer
  * @param offset Vị trí bắt đầu đọc
  * @param bytes_read Con trỏ đến biến lưu số bytes đã đọc
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int read_file_chunk(const char *file_path, char *buffer, size_t buffer_size, off_t offset, size_t *bytes_read);
 
 #endif /* FILE_PROCESS_H */