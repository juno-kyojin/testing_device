/**
 * @file log.c
 * @brief Triển khai các hàm log
 */

 #include "log.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <stdarg.h>
 #include <pthread.h>
 
 #define MAX_LOG_LINE_SIZE 2048
 
 // Cấu hình logger mặc định
 LoggerConfig logger_config = {
     .log_file_path = "application.log",
     .log_level = LOG_LVL_DEBUG
 };
 
 // Mutex để bảo vệ việc ghi log đồng thời
 static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
 
 // Tên của các log level
 static const char *log_level_names[] = {
     "NONE",
     "ERROR",
     "WARN",
     "DEBUG"
 };
 
 void init_logger(void) {
     // Mặc định, đảm bảo file log có thể ghi
     FILE *log_file = fopen(logger_config.log_file_path, "a");
     if (log_file) {
         fclose(log_file);
     } else {
         // Nếu không tạo được file log, sử dụng stderr
         fprintf(stderr, "Cannot open log file %s. Using stderr for logging.\n", logger_config.log_file_path);
     }
 }
 
 void cleanup_logger(void) {
     // Không có tài nguyên đặc biệt cần giải phóng
     // Mutex được khởi tạo tĩnh nên không cần destroy
 }
 
 void set_log_level(int level) {
     if (level >= LOG_LVL_NONE && level <= LOG_LVL_DEBUG) {
         logger_config.log_level = level;
     }
 }
 
 void set_log_file(const char *file_path) {
     if (file_path) {
         strncpy(logger_config.log_file_path, file_path, sizeof(logger_config.log_file_path) - 1);
         logger_config.log_file_path[sizeof(logger_config.log_file_path) - 1] = '\0';
         
         // Kiểm tra xem file có thể ghi được không
         FILE *log_file = fopen(logger_config.log_file_path, "a");
         if (log_file) {
             fclose(log_file);
         } else {
             fprintf(stderr, "Cannot open new log file %s. Using previous log file.\n", file_path);
         }
     }
 }
 
 void log_message(int level, const char *format, ...) {
     if (level <= LOG_LVL_NONE || level > LOG_LVL_DEBUG || level > logger_config.log_level) {
         return;
     }
     
  // Lấy thời gian hiện tại
  time_t now = time(NULL);
  struct tm tm_now;
  localtime_r(&now, &tm_now);
  
  char time_str[20];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_now);
  
  // Tạo chuỗi log
  char log_buffer[MAX_LOG_LINE_SIZE];
  
  // Header với timestamp và log level
  int header_len = snprintf(log_buffer, sizeof(log_buffer), "[%s] %s: ", 
                           time_str, log_level_names[level]);
  
  if (header_len < 0 || header_len >= sizeof(log_buffer)) {
      return; // Lỗi khi tạo header
  }
  
  // Nội dung log
  va_list args;
  va_start(args, format);
  int content_len = vsnprintf(log_buffer + header_len, sizeof(log_buffer) - header_len, 
                            format, args);
  va_end(args);
  
  if (content_len < 0) {
      return; // Lỗi khi tạo nội dung
  }
  
  // Đảm bảo có ký tự xuống dòng ở cuối
  size_t total_len = header_len + content_len;
  if (total_len < sizeof(log_buffer) - 2) {
      if (log_buffer[total_len - 1] != '\n') {
          log_buffer[total_len] = '\n';
          log_buffer[total_len + 1] = '\0';
      }
  } else {
      // Đặt ký tự xuống dòng cho chuỗi quá dài
      log_buffer[sizeof(log_buffer) - 2] = '\n';
      log_buffer[sizeof(log_buffer) - 1] = '\0';
  }
  
  pthread_mutex_lock(&log_mutex);
  
  // Ghi log vào file
  FILE *log_file = fopen(logger_config.log_file_path, "a");
  if (log_file) {
      fputs(log_buffer, log_file);
      fclose(log_file);
  } else {
      // Fallback: output to stderr if cannot open file
      fputs(log_buffer, stderr);
  }
  
  pthread_mutex_unlock(&log_mutex);
}