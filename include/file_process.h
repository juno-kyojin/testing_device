/**
 * @file file_process.h
 * @brief Định nghĩa các hàm xử lý file cơ bản (mở, đọc, ghi)
 */

 #ifndef FILE_PROCESS_H
 #define FILE_PROCESS_H
 
 #include <stdio.h>
 #include <stdbool.h>
 #include <stddef.h>
 #include <sys/types.h>
 
 /**
  * @brief Chế độ mở file
  */
 typedef enum {
     FILE_MODE_READ,       /**< Mở để đọc */
     FILE_MODE_WRITE,      /**< Mở để ghi (tạo mới/ghi đè) */
     FILE_MODE_APPEND,     /**< Mở để thêm vào cuối file */
     FILE_MODE_READ_WRITE  /**< Mở để đọc và ghi */
 } file_mode_t;
 
 /**
  * @brief Mở file với chế độ được chỉ định
  * 
  * @param filename Đường dẫn đến file
  * @param mode Chế độ mở file
  * @param max_retries Số lần thử lại tối đa nếu mở file thất bại
  * @return FILE* Con trỏ đến file đã mở hoặc NULL nếu có lỗi
  */
 FILE* open_file(const char* filename, file_mode_t mode, int max_retries);
 
 /**
  * @brief Đọc toàn bộ nội dung file vào buffer
  * 
  * @param filename Đường dẫn đến file
  * @param buffer Buffer lưu dữ liệu đọc được (được cấp phát động)
  * @param size Kích thước dữ liệu đọc được
  * @return int 0 nếu thành công, -1 nếu có lỗi
  */
 int read_file(const char* filename, char** buffer, size_t* size);
 
 /**
  * @brief Ghi dữ liệu vào file
  * 
  * @param filename Đường dẫn đến file
  * @param data Dữ liệu cần ghi
  * @param data_size Kích thước dữ liệu
  * @param append true để thêm vào cuối file, false để ghi đè
  * @return ssize_t Số byte đã ghi hoặc -1 nếu có lỗi
  */
 ssize_t write_to_file(const char* filename, const void* data, size_t data_size, bool append);
 
 #endif /* FILE_PROCESS_H */