/**
 * @file test_parser_data.c
 * @brief Kiểm thử chức năng xử lý và phân tích dữ liệu
 * @author juno-kyojin
 * @date 2025-03-19
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <assert.h>
 #include "parser_data.h"
 #include "file_process.h"
 #include "log.h"
 
 #define TEST_DATA_FILE "test_data.json"
 #define TEST_OUTPUT_FILE "test_output.json"
 
 /**
  * @brief Thiết lập môi trường kiểm thử
  */
 void setup_test_environment() {
     // Khởi tạo logger
     set_log_file("test_parser_data.log");
     set_log_level(LOG_LVL_DEBUG);
     init_logger();
     
     // Xóa các file test cũ nếu tồn tại
     if (file_exists(TEST_DATA_FILE)) delete_file(TEST_DATA_FILE);
     if (file_exists(TEST_OUTPUT_FILE)) delete_file(TEST_OUTPUT_FILE);
     
     printf("Môi trường kiểm thử đã được thiết lập.\n");
 }
 
 /**
  * @brief Dọn dẹp sau kiểm thử
  */
 void cleanup_test_environment() {
     // Xóa các file test
     if (file_exists(TEST_DATA_FILE)) delete_file(TEST_DATA_FILE);
     if (file_exists(TEST_OUTPUT_FILE)) delete_file(TEST_OUTPUT_FILE);
     
     // Dọn dẹp logger
     cleanup_logger();
     
     printf("Môi trường kiểm thử đã được dọn dẹp.\n");
 }
 
 /**
  * @brief Tạo file JSON dữ liệu test cases
  * @return 1 nếu thành công, 0 nếu thất bại
  */
 int create_test_json_file() {
     const char *content = "{\n"
                           "  \"test_cases\": [\n"
                           "    {\n"
                           "      \"id\": \"TC001\",\n"
                           "      \"type\": \"ping\",\n"
                           "      \"network\": \"LAN\",\n"
                           "      \"name\": \"Local Server Ping Test\",\n"
                           "      \"description\": \"Test connectivity to local server\",\n"
                           "      \"target\": \"192.168.1.100\",\n"
                           "      \"timeout\": 5000,\n"
                           "      \"enabled\": true,\n"
                           "      \"ping_params\": {\n"
                           "        \"count\": 5,\n"
                           "        \"size\": 64,\n"
                           "        \"interval\": 1000,\n"
                           "        \"ipv6\": false\n"
                           "      }\n"
                           "    },\n"
                           "    {\n"
                           "      \"id\": \"TC002\",\n"
                           "      \"type\": \"throughput\",\n"
                           "      \"network\": \"WAN\",\n"
                           "      \"name\": \"WAN Bandwidth Test\",\n"
                           "      \"description\": \"Test bandwidth to internet server\",\n"
                           "      \"target\": \"speed-test-server.example.com\",\n"
                           "      \"timeout\": 30000,\n"
                           "      \"enabled\": true,\n"
                           "      \"throughput_params\": {\n"
                           "        \"duration\": 10,\n"
                           "        \"protocol\": \"TCP\",\n"
                           "        \"port\": 5201\n"
                           "      }\n"
                           "    },\n"
                           "    {\n"
                           "      \"id\": \"TC003\",\n"
                           "      \"type\": \"security\",\n"
                           "      \"network\": \"BOTH\",\n"
                           "      \"name\": \"TLS Security Test\",\n"
                           "      \"description\": \"Test TLS security configuration\",\n"
                           "      \"target\": \"secure.example.com\",\n"
                           "      \"timeout\": 10000,\n"
                           "      \"enabled\": false,\n"
                           "      \"security_params\": {\n"
                           "        \"method\": \"tls_scan\",\n"
                           "        \"port\": 443\n"
                           "      }\n"
                           "    }\n"
                           "  ]\n"
                           "}\n";
     
     printf("Tạo file JSON test cases %s...\n", TEST_DATA_FILE);
     int result = write_file(TEST_DATA_FILE, content, strlen(content));
     if (result == 0) {
         printf("   ✓ Tạo file JSON thành công\n");
         return 1;
     } else {
         printf("   ✗ Không thể tạo file JSON\n");
         return 0;
     }
 }
 
 /**
  * @brief Kiểm tra chức năng đọc test cases từ file JSON
  */
 void test_read_json_test_cases() {
     printf("\n--- Kiểm tra đọc test cases từ file JSON ---\n");
     
     if (!create_test_json_file()) {
         printf("   ✗ Không thể tiếp tục kiểm thử vì không tạo được file JSON\n");
         return;
     }
     
     printf("1. Đọc test cases từ file JSON...\n");
     test_case_t *test_cases = NULL;
     int count = 0;
     
     bool success = read_json_test_cases(TEST_DATA_FILE, &test_cases, &count);
     
     if (success && test_cases != NULL && count > 0) {
         printf("   ✓ Đọc được %d test cases từ file JSON\n", count);
         
         // Kiểm tra số lượng test cases
         if (count == 3) {
             printf("   ✓ Số lượng test cases chính xác\n");
         } else {
             printf("   ✗ Số lượng test cases không chính xác, kỳ vọng: 3, thực tế: %d\n", count);
         }
         
         // Kiểm tra dữ liệu của test case đầu tiên
         printf("2. Kiểm tra dữ liệu test case đầu tiên (TC001)...\n");
         if (strcmp(test_cases[0].id, "TC001") == 0) {
             printf("   ✓ ID test case chính xác: %s\n", test_cases[0].id);
         } else {
             printf("   ✗ ID test case không chính xác, kỳ vọng: TC001, thực tế: %s\n", test_cases[0].id);
         }
         
         // Giải phóng bộ nhớ
         free_test_cases(test_cases, count);
     } else {
         printf("   ✗ Không thể đọc test cases từ file JSON\n");
     }
     
     printf("=> Kiểm tra đọc test cases từ file JSON hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra chức năng phân tích chuỗi JSON trực tiếp
  */
 void test_parse_json_content() {
     printf("\n--- Kiểm tra phân tích chuỗi JSON trực tiếp ---\n");
     
     const char *json_content = "{\n"
                                "  \"test_cases\": [\n"
                                "    {\n"
                                "      \"id\": \"TC004\",\n"
                                "      \"type\": \"other\",\n"
                                "      \"network\": \"LAN\",\n"
                                "      \"name\": \"Custom Test\",\n"
                                "      \"description\": \"A custom test case\",\n"
                                "      \"target\": \"localhost\",\n"
                                "      \"timeout\": 1000,\n"
                                "      \"enabled\": true\n"
                                "    }\n"
                                "  ]\n"
                                "}\n";
     
     printf("1. Phân tích chuỗi JSON trực tiếp...\n");
     test_case_t *test_cases = NULL;
     int count = 0;
     
     bool success = parse_json_content(json_content, &test_cases, &count);
     
     if (success && test_cases != NULL && count > 0) {
         printf("   ✓ Phân tích thành công %d test cases từ chuỗi JSON\n", count);
         
         // Kiểm tra dữ liệu của test case
         if (strcmp(test_cases[0].id, "TC004") == 0) {
             printf("   ✓ ID test case chính xác: %s\n", test_cases[0].id);
         } else {
             printf("   ✗ ID test case không chính xác, kỳ vọng: TC004, thực tế: %s\n", test_cases[0].id);
         }
         
         // Giải phóng bộ nhớ
         free_test_cases(test_cases, count);
     } else {
         printf("   ✗ Không thể phân tích chuỗi JSON\n");
     }
     
     printf("=> Kiểm tra phân tích chuỗi JSON trực tiếp hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra chức năng chuyển đổi test cases thành JSON
  */
 void test_test_cases_to_json() {
     printf("\n--- Kiểm tra chuyển đổi test cases thành JSON ---\n");
     
     if (!create_test_json_file()) {
         printf("   ✗ Không thể tiếp tục kiểm thử vì không tạo được file JSON\n");
         return;
     }
     
     printf("1. Đọc test cases và chuyển đổi lại thành JSON...\n");
     test_case_t *test_cases = NULL;
     int count = 0;
     
     // Đọc test cases
     bool read_success = read_json_test_cases(TEST_DATA_FILE, &test_cases, &count);
     
     if (read_success && test_cases != NULL && count > 0) {
         printf("   ✓ Đọc được %d test cases từ file JSON\n", count);
         
         // Chuyển đổi mảng test cases sang JSON
         char *json_buffer = (char*)malloc(10240); // 10KB buffer
         if (json_buffer == NULL) {
             printf("   ✗ Không thể cấp phát bộ nhớ cho buffer JSON\n");
             free_test_cases(test_cases, count);
             return;
         }
         
         bool convert_success = test_cases_to_json(test_cases, count, json_buffer, 10240);
         
         if (convert_success) {
             printf("   ✓ Chuyển đổi mảng test cases sang JSON thành công\n");
             
             // Ghi JSON vào file output để kiểm tra
             if (write_file(TEST_OUTPUT_FILE, json_buffer, strlen(json_buffer)) == 0) {
                 printf("   ✓ Ghi JSON vào file output thành công\n");
                 
                 // Hiển thị một phần nội dung để kiểm tra
                 printf("   - Một phần nội dung JSON:\n");
                 int preview_length = strlen(json_buffer) < 200 ? strlen(json_buffer) : 200;
                 char preview[201];
                 strncpy(preview, json_buffer, preview_length);
                 preview[preview_length] = '\0';
                 printf("%s...\n", preview);
             } else {
                 printf("   ✗ Không thể ghi JSON vào file output\n");
             }
         } else {
             printf("   ✗ Không thể chuyển đổi mảng test cases sang JSON\n");
         }
         
         // Giải phóng bộ nhớ
         free(json_buffer);
         free_test_cases(test_cases, count);
     } else {
         printf("   ✗ Không thể đọc test cases từ file JSON\n");
     }
     
     printf("=> Kiểm tra chuyển đổi test cases thành JSON hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra xử lý lỗi cơ bản
  */
 void test_error_handling() {
     printf("\n--- Kiểm tra xử lý lỗi cơ bản ---\n");
     
     printf("1. Đọc từ file JSON không tồn tại...\n");
     test_case_t *test_cases = NULL;
     int count = 0;
     
     bool success = read_json_test_cases("file_khong_ton_tai.json", &test_cases, &count);
     if (!success) {
         printf("   ✓ Xử lý đúng khi file không tồn tại\n");
     } else {
         printf("   ✗ Không xử lý đúng khi file không tồn tại\n");
         free_test_cases(test_cases, count);
     }
     
     printf("2. Phân tích chuỗi JSON không hợp lệ...\n");
     const char *invalid_json = "{ \"this\": \"is not valid JSON";
     
     success = parse_json_content(invalid_json, &test_cases, &count);
     if (!success) {
         printf("   ✓ Xử lý đúng khi chuỗi JSON không hợp lệ\n");
     } else {
         printf("   ✗ Không xử lý đúng khi chuỗi JSON không hợp lệ\n");
         free_test_cases(test_cases, count);
     }
     
     printf("=> Kiểm tra xử lý lỗi cơ bản hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra tích hợp với các module khác
  */
/**
 * @brief Kiểm tra tích hợp với các module khác
 */
void test_integration() {
    printf("\n--- Kiểm tra tích hợp với các module khác ---\n");
    
    printf("1. Tích hợp với file_process...\n");
    const char *json_content = "{\n"
                               "  \"test_cases\": [\n"
                               "    {\n"
                               "      \"id\": \"TC006\",\n"
                               "      \"type\": \"ping\",\n"
                               "      \"network\": \"LAN\",\n"
                               "      \"name\": \"Integration Test\",\n"
                               "      \"description\": \"Testing integration\",\n"
                               "      \"target\": \"localhost\",\n"
                               "      \"timeout\": 1000,\n"
                               "      \"enabled\": true,\n"
                               "      \"ping_params\": {\n"
                               "        \"count\": 3,\n"
                               "        \"size\": 32,\n"
                               "        \"interval\": 500,\n"
                               "        \"ipv6\": false\n"
                               "      }\n"
                               "    }\n"
                               "  ]\n"
                               "}\n";
    
    // Ghi file JSON bằng module file_process
    if (write_file(TEST_DATA_FILE, json_content, strlen(json_content)) == 0) {
        printf("   ✓ Ghi JSON vào file bằng module file_process thành công\n");
        
        // Đọc test cases bằng module parser_data
        test_case_t *test_cases = NULL;
        int count = 0;
        
        bool success = read_json_test_cases(TEST_DATA_FILE, &test_cases, &count);
        
        if (success && test_cases != NULL && count > 0) {
            printf("   ✓ Đọc test cases bằng module parser_data thành công\n");
            printf("   - Đọc được %d test case(s)\n", count);
            
            // Kiểm tra ID của test case
            if (count > 0 && strcmp(test_cases[0].id, "TC006") == 0) {
                printf("   ✓ ID test case chính xác\n");
            } else {
                printf("   ✗ ID test case không chính xác\n");
            }
            
            // Giải phóng bộ nhớ
            free_test_cases(test_cases, count);
        } else {
            printf("   ✗ Không thể đọc test cases\n");
        }
    } else {
        printf("   ✗ Không thể ghi JSON vào file\n");
    }
    
    printf("2. Tích hợp với log...\n");
    // Thiết lập log file mới để kiểm thử
    set_log_file("parser_integration.log");
    log_message(LOG_LVL_DEBUG, "Bắt đầu kiểm thử tích hợp log và parser_data");
    
    // Thực hiện một số thao tác parser và ghi log
    if (create_test_json_file()) {
        log_message(LOG_LVL_DEBUG, "Đã tạo file JSON test cases");
        
        test_case_t *test_cases = NULL;
        int count = 0;
        
        bool success = read_json_test_cases(TEST_DATA_FILE, &test_cases, &count);
        if (success && test_cases != NULL && count > 0) {
            log_message(LOG_LVL_DEBUG, "Đọc được %d test cases", count);
            
            for (int i = 0; i < count; i++) {
                log_message(LOG_LVL_DEBUG, "Test case %d: ID=%s, Name=%s", i+1, test_cases[i].id, test_cases[i].name);
            }
            
            free_test_cases(test_cases, count);
        } else {
            log_message(LOG_LVL_ERROR, "Không thể đọc test cases");
        }
    } else {
        log_message(LOG_LVL_ERROR, "Không thể tạo file JSON");
    }
    
    log_message(LOG_LVL_DEBUG, "Kết thúc kiểm thử tích hợp");
    
    // Kiểm tra log file
    if (file_exists("parser_integration.log")) {
        printf("   ✓ Log file đã được tạo\n");
        
        char *log_content = NULL;
        size_t log_size = 0;
        
        if (read_file("parser_integration.log", &log_content, &log_size) == 0) {
            printf("   ✓ Đọc log file thành công (%zu bytes)\n", log_size);
            
            const char *expected_msg = "Bắt đầu kiểm thử tích hợp log và parser_data";
            if (strstr(log_content, expected_msg)) {
                printf("   ✓ Log message được ghi chính xác\n");
            } else {
                printf("   ✗ Log message không được ghi chính xác\n");
            }
            
            free(log_content);
        } else {
            printf("   ✗ Không thể đọc log file\n");
        }
        
        // Dọn dẹp
        delete_file("parser_integration.log");
    } else {
        printf("   ✗ Log file không được tạo\n");
    }
    
    // Đặt lại log file mặc định
    set_log_file("test_parser_data.log");
    
    printf("=> Kiểm tra tích hợp với các module khác hoàn tất.\n");
}

/**
 * @brief Hàm main chạy tất cả các bài kiểm thử
 */
int main() {
    printf("\n=================================================\n");
    printf("      KIỂM THỬ MODULE PARSER_DATA.C\n");
    printf("=================================================\n");
    printf("Ngày giờ: %s\n", "2025-03-19 09:35:26");
    printf("Người dùng: %s\n", "juno-kyojin");
    printf("=================================================\n");
    
    // Thiết lập môi trường kiểm thử
    setup_test_environment();
    
    // Chạy các bài kiểm thử cho các hàm đã được triển khai
    test_read_json_test_cases();
    test_parse_json_content();
    test_test_cases_to_json();
    test_error_handling();
    test_integration();
    
    // Dọn dẹp môi trường kiểm thử
    cleanup_test_environment();
    
    printf("\n=================================================\n");
    printf("      HOÀN TẤT KIỂM THỬ PARSER_DATA.C\n");
    printf("=================================================\n");
    
    return 0;
}