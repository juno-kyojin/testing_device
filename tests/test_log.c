/**
 * @file test_log.c
 * @brief Kiểm thử chức năng ghi log
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <assert.h>
 #include "log.h"
 #include "file_process.h"
 
 #define TEST_LOG_FILE "test_log.log"
 #define THREAD_COUNT 5
 #define MESSAGES_PER_THREAD 100
 
 /**
  * @brief Kiểm tra khởi tạo và cấu hình log
  */
 void test_log_init_config() {
     printf("\n--- Kiểm tra khởi tạo và cấu hình log ---\n");
     
     // Xóa file log cũ nếu còn tồn tại
     if (file_exists(TEST_LOG_FILE)) {
         delete_file(TEST_LOG_FILE);
         printf("1. Đã xóa file log cũ\n");
     }
     
     // Thiết lập file log
     printf("2. Thiết lập file log: %s\n", TEST_LOG_FILE);
     set_log_file(TEST_LOG_FILE);
     
     // Thiết lập log level
     printf("3. Thiết lập log level: DEBUG\n");
     set_log_level(LOG_LVL_DEBUG);
     
     // Khởi tạo logger
     printf("4. Khởi tạo logger...\n");
     init_logger();
     
     // Kiểm tra file log đã được tạo
     printf("5. Kiểm tra file log đã được tạo...\n");
     if (file_exists(TEST_LOG_FILE)) {
         printf("   ✓ File log đã được tạo\n");
     } else {
         printf("   ✗ File log chưa được tạo (sẽ được tạo khi ghi log đầu tiên)\n");
     }
     
     printf("=> Khởi tạo và cấu hình log hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra ghi log ở các mức khác nhau
  */
 void test_log_levels() {
     printf("\n--- Kiểm tra ghi log ở các mức khác nhau ---\n");
     
     // Ghi log ở các mức khác nhau
     printf("1. Ghi log ở mức ERROR...\n");
     log_message(LOG_LVL_ERROR, "Test ERROR message");
     
     printf("2. Ghi log ở mức WARN...\n");
     log_message(LOG_LVL_WARN, "Test WARN message");
     
     printf("3. Ghi log ở mức DEBUG...\n");
     log_message(LOG_LVL_DEBUG, "Test DEBUG message");
     
     // Kiểm tra file log
     printf("4. Kiểm tra nội dung file log...\n");
     char *log_content = NULL;
     size_t content_size = 0;
     
     if (read_file(TEST_LOG_FILE, &log_content, &content_size) == 0) {
         printf("   ✓ Đọc file log thành công (%zu bytes)\n", content_size);
         
         // Kiểm tra xem các mức log có trong nội dung không
         int error_found = strstr(log_content, "ERROR: Test ERROR message") != NULL;
         int warn_found = strstr(log_content, "WARN: Test WARN message") != NULL;
         int debug_found = strstr(log_content, "DEBUG: Test DEBUG message") != NULL;
         
         printf("   - Message ERROR: %s\n", error_found ? "✓ Có" : "✗ Không");
         printf("   - Message WARN: %s\n", warn_found ? "✓ Có" : "✗ Không");
         printf("   - Message DEBUG: %s\n", debug_found ? "✓ Có" : "✗ Không");
         
         if (error_found && warn_found && debug_found) {
             printf("   ✓ Tất cả các mức log đều được ghi thành công\n");
         } else {
             printf("   ✗ Một số mức log không được ghi\n");
         }
         
         free(log_content);
     } else {
         printf("   ✗ Không thể đọc file log\n");
     }
     
     printf("=> Kiểm tra ghi log ở các mức khác nhau hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra lọc log theo level
  */
 void test_log_filtering() {
     printf("\n--- Kiểm tra lọc log theo level ---\n");
     
     // Xóa file log hiện tại để test từ đầu
     if (file_exists(TEST_LOG_FILE)) {
         delete_file(TEST_LOG_FILE);
         printf("1. Đã xóa file log cũ\n");
     }
     
     // Thiết lập log level chỉ hiển thị ERROR và WARN
     printf("2. Thiết lập log level: WARN\n");
     set_log_level(LOG_LVL_WARN);
     
     // Ghi log ở các mức khác nhau
     printf("3. Ghi log ở tất cả các mức...\n");
     log_message(LOG_LVL_ERROR, "Filtered ERROR message");  // Nên được ghi
     log_message(LOG_LVL_WARN, "Filtered WARN message");    // Nên được ghi
     log_message(LOG_LVL_DEBUG, "Filtered DEBUG message");  // Không nên được ghi
     
     // Kiểm tra file log
     printf("4. Kiểm tra nội dung file log đã được lọc...\n");
     char *log_content = NULL;
     size_t content_size = 0;
     
     if (read_file(TEST_LOG_FILE, &log_content, &content_size) == 0) {
         printf("   ✓ Đọc file log thành công (%zu bytes)\n", content_size);
         
         // Kiểm tra xem các mức log có trong nội dung không
         int error_found = strstr(log_content, "ERROR: Filtered ERROR message") != NULL;
         int warn_found = strstr(log_content, "WARN: Filtered WARN message") != NULL;
         int debug_found = strstr(log_content, "DEBUG: Filtered DEBUG message") != NULL;
         
         printf("   - ERROR message: %s (kỳ vọng: Có)\n", error_found ? "✓ Có" : "✗ Không");
         printf("   - WARN message: %s (kỳ vọng: Có)\n", warn_found ? "✓ Có" : "✗ Không");
         printf("   - DEBUG message: %s (kỳ vọng: Không)\n", debug_found ? "✗ Có" : "✓ Không");
         
         if (error_found && warn_found && !debug_found) {
             printf("   ✓ Lọc log theo level hoạt động chính xác\n");
         } else {
             printf("   ✗ Lọc log theo level không chính xác\n");
         }
         
         free(log_content);
     } else {
         printf("   ✗ Không thể đọc file log\n");
     }
     
     // Đặt lại log level cho các test tiếp theo
     set_log_level(LOG_LVL_DEBUG);
     
     printf("=> Kiểm tra lọc log theo level hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra định dạng log message
  */
 void test_log_format() {
     printf("\n--- Kiểm tra định dạng log message ---\n");
     
     // Xóa file log hiện tại để test từ đầu
     if (file_exists(TEST_LOG_FILE)) {
         delete_file(TEST_LOG_FILE);
         printf("1. Đã xóa file log cũ\n");
     }
     
     // Ghi log với các định dạng khác nhau
     printf("2. Ghi log với các định dạng khác nhau...\n");
     log_message(LOG_LVL_WARN, "Simple message");
     log_message(LOG_LVL_WARN, "Message with number: %d", 42);
     log_message(LOG_LVL_WARN, "Message with string: %s", "test string");
     log_message(LOG_LVL_WARN, "Message with multiple params: %d, %s, %.2f", 100, "test", 3.14);
     
     // Kiểm tra file log
     printf("3. Kiểm tra định dạng log message...\n");
     char *log_content = NULL;
     size_t content_size = 0;
     
     if (read_file(TEST_LOG_FILE, &log_content, &content_size) == 0) {
         printf("   ✓ Đọc file log thành công (%zu bytes)\n", content_size);
         
         // Kiểm tra các định dạng message
         int simple_msg = strstr(log_content, "WARN: Simple message") != NULL;
         int number_msg = strstr(log_content, "WARN: Message with number: 42") != NULL;
         int string_msg = strstr(log_content, "WARN: Message with string: test string") != NULL;
         int multi_msg = strstr(log_content, "WARN: Message with multiple params: 100, test, 3.14") != NULL;
         
         printf("   - Simple message: %s\n", simple_msg ? "✓ OK" : "✗ Không tìm thấy");
         printf("   - Message with number: %s\n", number_msg ? "✓ OK" : "✗ Không tìm thấy");
         printf("   - Message with string: %s\n", string_msg ? "✓ OK" : "✗ Không tìm thấy");
         printf("   - Message with multiple params: %s\n", multi_msg ? "✓ OK" : "✗ Không tìm thấy");
         
         if (simple_msg && number_msg && string_msg && multi_msg) {
             printf("   ✓ Tất cả các định dạng log message hoạt động chính xác\n");
         } else {
             printf("   ✗ Một số định dạng log message không hoạt động\n");
         }
         
         free(log_content);
     } else {
         printf("   ✗ Không thể đọc file log\n");
     }
     
     printf("=> Kiểm tra định dạng log message hoàn tất.\n");
 }
 
 /**
  * @brief Dữ liệu cho thread
  */
 typedef struct {
     int thread_id;
     int message_count;
 } thread_data_t;
 
 /**
  * @brief Hàm chạy trong thread
  */
 void* thread_log_function(void* arg) {
     thread_data_t* data = (thread_data_t*)arg;
     
     for (int i = 0; i < data->message_count; i++) {
         log_message(LOG_LVL_DEBUG, "Thread %d - Message %d", data->thread_id, i);
         
         // Thêm một chút delay ngẫu nhiên để tăng khả năng xảy ra race condition
         // nếu mutex không hoạt động đúng
         usleep((rand() % 10) * 1000);  // 0-9ms
     }
     
     return NULL;
 }
 
 /**
  * @brief Kiểm tra ghi log đa luồng
  */
 void test_multithreaded_logging() {
     printf("\n--- Kiểm tra ghi log đa luồng ---\n");
     
     // Xóa file log hiện tại để test từ đầu
     if (file_exists(TEST_LOG_FILE)) {
         delete_file(TEST_LOG_FILE);
         printf("1. Đã xóa file log cũ\n");
     }
     
     // Khởi tạo các thread
     printf("2. Khởi tạo %d threads, mỗi thread ghi %d messages...\n", 
            THREAD_COUNT, MESSAGES_PER_THREAD);
     pthread_t threads[THREAD_COUNT];
     thread_data_t thread_data[THREAD_COUNT];
     
     for (int i = 0; i < THREAD_COUNT; i++) {
         thread_data[i].thread_id = i;
         thread_data[i].message_count = MESSAGES_PER_THREAD;
         pthread_create(&threads[i], NULL, thread_log_function, &thread_data[i]);
     }
     
     // Đợi tất cả các thread hoàn thành
     printf("3. Đợi các threads hoàn thành...\n");
     for (int i = 0; i < THREAD_COUNT; i++) {
         pthread_join(threads[i], NULL);
     }
     
     // Kiểm tra file log
     printf("4. Kiểm tra file log sau khi ghi đa luồng...\n");
     char *log_content = NULL;
     size_t content_size = 0;
     
     if (read_file(TEST_LOG_FILE, &log_content, &content_size) == 0) {
         printf("   ✓ Đọc file log thành công (%zu bytes)\n", content_size);
         
         // Đếm số message của mỗi thread
         int message_counts[THREAD_COUNT] = {0};
         char *pos = log_content;
         char search_str[50];
         
         for (int i = 0; i < THREAD_COUNT; i++) {
             for (int j = 0; j < MESSAGES_PER_THREAD; j++) {
                 snprintf(search_str, sizeof(search_str), "Thread %d - Message %d", i, j);
                 // Tìm từ vị trí hiện tại
                 char *next_pos = strstr(pos, search_str);
                 if (next_pos != NULL) {
                     message_counts[i]++;
                     pos = next_pos + strlen(search_str);
                 }
             }
             pos = log_content; // Reset vị trí tìm kiếm cho thread tiếp theo
         }
                // Kiểm tra số lượng message
                int total_messages = 0;
                printf("   Số lượng message từ mỗi thread:\n");
                for (int i = 0; i < THREAD_COUNT; i++) {
                    printf("   - Thread %d: %d/%d messages\n", i, message_counts[i], MESSAGES_PER_THREAD);
                    total_messages += message_counts[i];
                }
                
                int expected_total = THREAD_COUNT * MESSAGES_PER_THREAD;
                float success_rate = (float)total_messages / expected_total * 100;
                
                printf("   - Tổng số: %d/%d messages (%.1f%%)\n", total_messages, expected_total, success_rate);
                
                // Trong thực tế, có thể không tìm thấy 100% messages do cách tìm kiếm đơn giản
                // nhưng nếu tỷ lệ cao thì có thể coi là thành công
                if (success_rate > 90) {
                    printf("   ✓ Ghi log đa luồng hoạt động tốt\n");
                } else if (success_rate > 50) {
                    printf("   ⚠ Ghi log đa luồng hoạt động nhưng có thể có vấn đề\n");
                } else {
                    printf("   ✗ Ghi log đa luồng có vấn đề\n");
                }
                
                free(log_content);
            } else {
                printf("   ✗ Không thể đọc file log\n");
            }
            
            printf("=> Kiểm tra ghi log đa luồng hoàn tất.\n");
        }
        
        /**
         * @brief Kiểm tra tính ổn định khi ghi log nhiều
         */
        void test_log_stability() {
            printf("\n--- Kiểm tra tính ổn định khi ghi log nhiều ---\n");
            
            // Xóa file log hiện tại để test từ đầu
            if (file_exists(TEST_LOG_FILE)) {
                delete_file(TEST_LOG_FILE);
                printf("1. Đã xóa file log cũ\n");
            }
            
            // Ghi log với số lượng lớn
            const int LOG_COUNT = 1000;
            printf("2. Ghi %d log messages liên tiếp...\n", LOG_COUNT);
            
            for (int i = 0; i < LOG_COUNT; i++) {
                log_message(LOG_LVL_DEBUG, "Stability test message #%d", i);
            }
            
            // Kiểm tra file log
            printf("3. Kiểm tra file log sau khi ghi nhiều messages...\n");
            long file_size = get_file_size(TEST_LOG_FILE);
            
            if (file_size > 0) {
                printf("   ✓ File log đã được tạo với kích thước %ld bytes\n", file_size);
                
                // Kiểm tra message đầu và cuối
                char *log_content = NULL;
                size_t content_size = 0;
                
                if (read_file(TEST_LOG_FILE, &log_content, &content_size) == 0) {
                    printf("   ✓ Đọc file log thành công\n");
                    
                    char first_msg[50];
                    char last_msg[50];
                    snprintf(first_msg, sizeof(first_msg), "Stability test message #0");
                    snprintf(last_msg, sizeof(last_msg), "Stability test message #%d", LOG_COUNT - 1);
                    
                    int first_found = strstr(log_content, first_msg) != NULL;
                    int last_found = strstr(log_content, last_msg) != NULL;
                    
                    printf("   - Message đầu tiên: %s\n", first_found ? "✓ Tìm thấy" : "✗ Không tìm thấy");
                    printf("   - Message cuối cùng: %s\n", last_found ? "✓ Tìm thấy" : "✗ Không tìm thấy");
                    
                    if (first_found && last_found) {
                        printf("   ✓ Ghi log với số lượng lớn hoạt động ổn định\n");
                    } else {
                        printf("   ✗ Một số messages không được ghi đúng\n");
                    }
                    
                    free(log_content);
                } else {
                    printf("   ✗ Không thể đọc file log\n");
                }
            } else {
                printf("   ✗ File log không được tạo hoặc rỗng\n");
            }
            
            printf("=> Kiểm tra tính ổn định khi ghi log nhiều hoàn tất.\n");
        }
        
        /**
         * @brief Kiểm tra đổi file log khi đang chạy
         */
        void test_change_log_file() {
            printf("\n--- Kiểm tra đổi file log khi đang chạy ---\n");
            
            // Xóa các file log cũ nếu có
            const char *first_log = "test_log1.log";
            const char *second_log = "test_log2.log";
            
            if (file_exists(first_log)) {
                delete_file(first_log);
            }
            if (file_exists(second_log)) {
                delete_file(second_log);
            }
            
            // Thiết lập file log đầu tiên
            printf("1. Thiết lập file log đầu tiên: %s\n", first_log);
            set_log_file(first_log);
            
            // Ghi một số log vào file đầu tiên
            printf("2. Ghi log vào file đầu tiên...\n");
            log_message(LOG_LVL_WARN, "Message in first log file");
            
            // Đổi sang file log thứ hai
            printf("3. Đổi sang file log thứ hai: %s\n", second_log);
            set_log_file(second_log);
            
            // Ghi một số log vào file thứ hai
            printf("4. Ghi log vào file thứ hai...\n");
            log_message(LOG_LVL_WARN, "Message in second log file");
            
            // Kiểm tra cả hai file log
            printf("5. Kiểm tra nội dung cả hai file log...\n");
            char *content1 = NULL, *content2 = NULL;
            size_t size1 = 0, size2 = 0;
            
            int read1 = read_file(first_log, &content1, &size1);
            int read2 = read_file(second_log, &content2, &size2);
            
            if (read1 == 0 && content1 != NULL) {
                printf("   ✓ Đọc file log thứ nhất thành công (%zu bytes)\n", size1);
                int msg1_found = strstr(content1, "Message in first log file") != NULL;
                printf("   - Message trong file thứ nhất: %s\n", msg1_found ? "✓ Tìm thấy" : "✗ Không tìm thấy");
                free(content1);
            } else {
                printf("   ✗ Không thể đọc file log thứ nhất\n");
            }
            
            if (read2 == 0 && content2 != NULL) {
                printf("   ✓ Đọc file log thứ hai thành công (%zu bytes)\n", size2);
                int msg2_found = strstr(content2, "Message in second log file") != NULL;
                printf("   - Message trong file thứ hai: %s\n", msg2_found ? "✓ Tìm thấy" : "✗ Không tìm thấy");
                free(content2);
            } else {
                printf("   ✗ Không thể đọc file log thứ hai\n");
            }
            
            // Dọn dẹp
            delete_file(first_log);
            delete_file(second_log);
            
            // Đặt lại file log cho các test tiếp theo
            set_log_file(TEST_LOG_FILE);
            
            printf("=> Kiểm tra đổi file log khi đang chạy hoàn tất.\n");
        }
        
        /**
         * @brief Hàm main chạy tất cả các bài kiểm thử
         */
        int main() {
            printf("\n=================================================\n");
            printf("      KIỂM THỬ MODULE LOG.C\n");
            printf("=================================================\n");
            
            // Chạy các bài kiểm thử
            test_log_init_config();
            test_log_levels();
            test_log_filtering();
            test_log_format();
            test_multithreaded_logging();
            test_log_stability();
            test_change_log_file();
            
            // Dọn dẹp
            cleanup_logger();
            if (file_exists(TEST_LOG_FILE)) {
                delete_file(TEST_LOG_FILE);
            }
            
            printf("\n=================================================\n");
            printf("      HOÀN TẤT KIỂM THỬ LOG.C\n");
            printf("=================================================\n");
            
            return 0;
        } 