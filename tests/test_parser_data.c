/**
 * @file test_parser_data.c
 * @brief Kiểm thử chức năng xử lý và phân tích dữ liệu
 * @author juno-kyojin
 * @date 2025-03-20
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
 #define TEST_DEFAULT_FILE "test_default.json"
 
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
     if (file_exists(TEST_DEFAULT_FILE)) delete_file(TEST_DEFAULT_FILE);
     
     printf("Môi trường kiểm thử đã được thiết lập.\n");
 }
 
 /**
  * @brief Dọn dẹp sau kiểm thử
  */
 void cleanup_test_environment() {
     // Xóa các file test
     if (file_exists(TEST_DATA_FILE)) delete_file(TEST_DATA_FILE);
     if (file_exists(TEST_OUTPUT_FILE)) delete_file(TEST_OUTPUT_FILE);
     if (file_exists(TEST_DEFAULT_FILE)) delete_file(TEST_DEFAULT_FILE);
     
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
  * @brief Tạo file JSON với một số trường bị thiếu để kiểm tra giá trị mặc định
  * @return 1 nếu thành công, 0 nếu thất bại
  */
 int create_test_json_with_missing_fields() {
     const char *content = "{\n"
                           "  \"test_cases\": [\n"
                           "    {\n"
                           "      \"id\": \"TC005\",\n"
                           "      \"type\": \"ping\",\n"
                           "      \"name\": \"Test With Missing Fields\"\n"
                           "    },\n"
                           "    {\n"
                           "      \"name\": \"Missing ID Test\",\n"
                           "      \"type\": \"throughput\",\n"
                           "      \"target\": \"example.com\"\n"
                           "    },\n"
                           "    {\n"
                           "      \"id\": \"TC007\"\n"
                           "    }\n"
                           "  ]\n"
                           "}\n";
     
     printf("Tạo file JSON với trường bị thiếu %s...\n", TEST_DEFAULT_FILE);
     int result = write_file(TEST_DEFAULT_FILE, content, strlen(content));
     if (result == 0) {
         printf("   ✓ Tạo file JSON với trường bị thiếu thành công\n");
         return 1;
     } else {
         printf("   ✗ Không thể tạo file JSON với trường bị thiếu\n");
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
         
         // Kiểm tra các trường cơ bản
         if (strcmp(test_cases[0].name, "Local Server Ping Test") == 0) {
             printf("   ✓ Tên test case chính xác\n");
         } else {
             printf("   ✗ Tên test case không chính xác\n");
         }
         
         if (strcmp(test_cases[0].description, "Test connectivity to local server") == 0) {
             printf("   ✓ Mô tả test case chính xác\n");
         } else {
             printf("   ✗ Mô tả test case không chính xác, kỳ vọng: 'Test connectivity to local server', thực tế: '%s'\n", 
                    test_cases[0].description);
         }
         
         if (strcmp(test_cases[0].target, "192.168.1.100") == 0) {
             printf("   ✓ Target test case chính xác\n");
         } else {
             printf("   ✗ Target test case không chính xác, kỳ vọng: '192.168.1.100', thực tế: '%s'\n", 
                    test_cases[0].target);
         }
         
         if (test_cases[0].timeout == 5000) {
             printf("   ✓ Timeout test case chính xác\n");
         } else {
             printf("   ✗ Timeout test case không chính xác, kỳ vọng: 5000, thực tế: %d\n", 
                    test_cases[0].timeout);
         }
         
         if (test_cases[0].enabled) {
             printf("   ✓ Trạng thái enabled test case chính xác\n");
         } else {
             printf("   ✗ Trạng thái enabled test case không chính xác, kỳ vọng: true, thực tế: false\n");
         }
         
         // Kiểm tra loại test và tham số đặc thù
         if (test_cases[0].type == TEST_PING) {
             printf("   ✓ Loại test case chính xác: PING\n");
             
             // Kiểm tra tham số ping
             if (test_cases[0].params.ping.count == 5) {
                 printf("   ✓ Tham số ping.count chính xác\n");
             } else {
                 printf("   ✗ Tham số ping.count không chính xác, kỳ vọng: 5, thực tế: %d\n", 
                        test_cases[0].params.ping.count);
             }
             
             if (test_cases[0].params.ping.size == 64) {
                 printf("   ✓ Tham số ping.size chính xác\n");
             } else {
                 printf("   ✗ Tham số ping.size không chính xác, kỳ vọng: 64, thực tế: %d\n", 
                        test_cases[0].params.ping.size);
             }
             
             if (test_cases[0].params.ping.interval == 1000) {
                 printf("   ✓ Tham số ping.interval chính xác\n");
             } else {
                 printf("   ✗ Tham số ping.interval không chính xác, kỳ vọng: 1000, thực tế: %d\n", 
                        test_cases[0].params.ping.interval);
             }
             
             if (!test_cases[0].params.ping.ipv6) {
                 printf("   ✓ Tham số ping.ipv6 chính xác\n");
             } else {
                 printf("   ✗ Tham số ping.ipv6 không chính xác, kỳ vọng: false, thực tế: true\n");
             }
         } else {
             printf("   ✗ Loại test case không chính xác, kỳ vọng: PING\n");
         }
         
         // Kiểm tra loại mạng
         if (test_cases[0].network_type == NETWORK_LAN) {
             printf("   ✓ Loại mạng test case chính xác: LAN\n");
         } else {
             printf("   ✗ Loại mạng test case không chính xác, kỳ vọng: LAN\n");
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
         
         // Kiểm tra các trường cơ bản mới
         if (strcmp(test_cases[0].description, "A custom test case") == 0) {
             printf("   ✓ Mô tả test case chính xác\n");
         } else {
             printf("   ✗ Mô tả test case không chính xác, kỳ vọng: 'A custom test case', thực tế: '%s'\n", 
                    test_cases[0].description);
         }
         
         if (strcmp(test_cases[0].target, "localhost") == 0) {
             printf("   ✓ Target test case chính xác\n");
         } else {
             printf("   ✗ Target test case không chính xác, kỳ vọng: 'localhost', thực tế: '%s'\n", 
                    test_cases[0].target);
         }
         
         if (test_cases[0].timeout == 1000) {
             printf("   ✓ Timeout test case chính xác\n");
         } else {
             printf("   ✗ Timeout test case không chính xác, kỳ vọng: 1000, thực tế: %d\n", 
                    test_cases[0].timeout);
         }
         
         if (test_cases[0].enabled) {
             printf("   ✓ Trạng thái enabled test case chính xác\n");
         } else {
             printf("   ✗ Trạng thái enabled test case không chính xác, kỳ vọng: true, thực tế: false\n");
         }
         
         // Giải phóng bộ nhớ
         free_test_cases(test_cases, count);
     } else {
         printf("   ✗ Không thể phân tích chuỗi JSON\n");
     }
     
     printf("=> Kiểm tra phân tích chuỗi JSON trực tiếp hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra xử lý giá trị mặc định khi các trường bị thiếu
  */
 void test_default_values() {
     printf("\n--- Kiểm tra xử lý giá trị mặc định ---\n");
     
     if (!create_test_json_with_missing_fields()) {
         printf("   ✗ Không thể tiếp tục kiểm thử vì không tạo được file JSON\n");
         return;
     }
     
     test_case_t *test_cases = NULL;
     int count = 0;
     
     printf("1. Đọc test cases từ file với các trường bị thiếu...\n");
     bool success = read_json_test_cases(TEST_DEFAULT_FILE, &test_cases, &count);
     
     if (success && test_cases != NULL && count > 0) {
         printf("   ✓ Đọc được %d test cases từ file JSON\n", count);
         
         // Kiểm tra TC005 với description, target, timeout và enabled bị thiếu
         printf("2. Kiểm tra các giá trị mặc định cho TC005...\n");
         test_case_t *tc5 = NULL;
         for (int i = 0; i < count; i++) {
             if (strcmp(test_cases[i].id, "TC005") == 0) {
                 tc5 = &test_cases[i];
                 break;
             }
         }
         
         if (tc5) {
             printf("   ✓ Tìm thấy test case TC005\n");
             
             // Description mặc định là rỗng
             if (strlen(tc5->description) == 0) {
                 printf("   ✓ Mô tả mặc định là rỗng\n");
             } else {
                 printf("   ✗ Mô tả mặc định không đúng, kỳ vọng: '', thực tế: '%s'\n", tc5->description);
             }
             
             // Target mặc định là rỗng
             if (strlen(tc5->target) == 0) {
                 printf("   ✓ Target mặc định là rỗng\n");
             } else {
                 printf("   ✗ Target mặc định không đúng, kỳ vọng: '', thực tế: '%s'\n", tc5->target);
             }
             
             // Timeout mặc định là 10000
             if (tc5->timeout == 10000) {
                 printf("   ✓ Timeout mặc định là 10000ms\n");
             } else {
                 printf("   ✗ Timeout mặc định không đúng, kỳ vọng: 10000, thực tế: %d\n", tc5->timeout);
             }
             
             // Enabled mặc định là true
             if (tc5->enabled) {
                 printf("   ✓ Trạng thái enabled mặc định là true\n");
             } else {
                 printf("   ✗ Trạng thái enabled mặc định không đúng, kỳ vọng: true, thực tế: false\n");
             }
             
             // Network mặc định là LAN
             if (tc5->network_type == NETWORK_LAN) {
                 printf("   ✓ Loại mạng mặc định là LAN\n");
             } else {
                 printf("   ✗ Loại mạng mặc định không đúng, kỳ vọng: LAN\n");
             }
             
             // Kiểm tra tham số ping mặc định
             if (tc5->type == TEST_PING) {
                 printf("   ✓ Loại test là PING\n");
                 
                 if (tc5->params.ping.count == 4) {
                     printf("   ✓ Tham số ping.count mặc định là 4\n");
                 } else {
                     printf("   ✗ Tham số ping.count mặc định không đúng, kỳ vọng: 4, thực tế: %d\n", 
                           tc5->params.ping.count);
                 }
                 
                 if (tc5->params.ping.size == 64) {
                     printf("   ✓ Tham số ping.size mặc định là 64\n");
                 } else {
                     printf("   ✗ Tham số ping.size mặc định không đúng, kỳ vọng: 64, thực tế: %d\n", 
                           tc5->params.ping.size);
                 }
             }
         } else {
             printf("   ✗ Không tìm thấy test case TC005\n");
         }
         
         // Kiểm tra test case không có ID
         printf("3. Kiểm tra ID mặc định cho test case thiếu ID...\n");
         bool found_missing_id = false;
         for (int i = 0; i < count; i++) {
             if (strncmp(test_cases[i].id, "TC", 2) == 0 && 
                 test_cases[i].id[2] >= '0' && test_cases[i].id[2] <= '9' &&
                 strcmp(test_cases[i].name, "Missing ID Test") == 0) {
                 found_missing_id = true;
                 printf("   ✓ ID mặc định được tạo: %s\n", test_cases[i].id);
                 break;
             }
         }
         
         if (!found_missing_id) {
             printf("   ✗ Không tìm thấy test case với ID mặc định cho 'Missing ID Test'\n");
         }
         
         // Giải phóng bộ nhớ
         free_test_cases(test_cases, count);
     } else {
         printf("   ✗ Không thể đọc test cases từ file JSON\n");
     }
     
     printf("=> Kiểm tra xử lý giá trị mặc định hoàn tất.\n");
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
                 
                 // Đọc lại JSON đã xuất để kiểm tra tính nhất quán
                 printf("2. Kiểm tra tính nhất quán của JSON sau khi chuyển đổi...\n");
                 test_case_t *test_cases2 = NULL;
                 int count2 = 0;
                 
                 bool parse_success = parse_json_content(json_buffer, &test_cases2, &count2);
                 
                 if (parse_success && test_cases2 != NULL && count2 > 0) {
                     printf("   ✓ Phân tích lại JSON thành công, đọc được %d test cases\n", count2);
                     
                     // So sánh số lượng test cases
                     if (count == count2) {
                         printf("   ✓ Số lượng test cases nhất quán\n");
                     } else {
                         printf("   ✗ Số lượng test cases không nhất quán, ban đầu: %d, sau chuyển đổi: %d\n", 
                               count, count2);
                     }
                     
                     // So sánh ID test case đầu tiên
                     if (strcmp(test_cases[0].id, test_cases2[0].id) == 0) {
                         printf("   ✓ ID test case nhất quán\n");
                     } else {
                         printf("   ✗ ID test case không nhất quán, ban đầu: %s, sau chuyển đổi: %s\n", 
                               test_cases[0].id, test_cases2[0].id);
                     }
                     
                     // Giải phóng bộ nhớ
                     free_test_cases(test_cases2, count2);
                 } else {
                     printf("   ✗ Không thể phân tích lại JSON sau khi chuyển đổi\n");
                 }
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
    
    printf("3. Truyền tham số không hợp lệ cho hàm parse_json_content...\n");
    success = parse_json_content(NULL, &test_cases, &count);
    if (!success) {
        printf("   ✓ Xử lý đúng khi tham số json_content là NULL\n");
    } else {
        printf("   ✗ Không xử lý đúng khi tham số json_content là NULL\n");
        free_test_cases(test_cases, count);
    }
    
    const char *valid_json = "{ \"test_cases\": [] }";
    success = parse_json_content(valid_json, NULL, &count);
    if (!success) {
        printf("   ✓ Xử lý đúng khi tham số test_cases là NULL\n");
    } else {
        printf("   ✗ Không xử lý đúng khi tham số test_cases là NULL\n");
    }
    
    success = parse_json_content(valid_json, &test_cases, NULL);
    if (!success) {
        printf("   ✓ Xử lý đúng khi tham số count là NULL\n");
    } else {
        printf("   ✗ Không xử lý đúng khi tham số count là NULL\n");
        free_test_cases(test_cases, count);
    }
    
    printf("=> Kiểm tra xử lý lỗi cơ bản hoàn tất.\n");
}

/**
 * @brief Kiểm tra xử lý extra_data
 */
void test_extra_data() {
    printf("\n--- Kiểm tra xử lý extra_data ---\n");
    
    const char *json_with_extra = "{\n"
                                  "  \"test_cases\": [\n"
                                  "    {\n"
                                  "      \"id\": \"TC008\",\n"
                                  "      \"name\": \"Test with Extra Data\",\n"
                                  "      \"type\": \"other\",\n"
                                  "      \"extra_data\": {\n"
                                  "        \"custom_field1\": \"value1\",\n"
                                  "        \"custom_field2\": 42,\n"
                                  "        \"custom_array\": [1, 2, 3],\n"
                                  "        \"custom_object\": {\n"
                                  "          \"nested\": true\n"
                                  "        }\n"
                                  "      }\n"
                                  "    }\n"
                                  "  ]\n"
                                  "}\n";
    
    printf("1. Phân tích JSON với extra_data...\n");
    test_case_t *test_cases = NULL;
    int count = 0;
    
    bool success = parse_json_content(json_with_extra, &test_cases, &count);
    
    if (success && test_cases != NULL && count > 0) {
        printf("   ✓ Phân tích thành công JSON với extra_data\n");
        
        // Kiểm tra ID
        if (strcmp(test_cases[0].id, "TC008") == 0) {
            printf("   ✓ ID test case chính xác: %s\n", test_cases[0].id);
        } else {
            printf("   ✗ ID test case không chính xác, kỳ vọng: TC008, thực tế: %s\n", test_cases[0].id);
        }
        
        // Kiểm tra extra_data
        if (test_cases[0].extra_data != NULL) {
            printf("   ✓ extra_data được trích xuất\n");
            
            // Kiểm tra chuỗi JSON trong extra_data có chứa các giá trị kỳ vọng
            const char *expected_values[] = {"custom_field1", "value1", "custom_field2", "42", "custom_array", "nested", "true"};
            int found_count = 0;
            
            for (int i = 0; i < sizeof(expected_values) / sizeof(expected_values[0]); i++) {
                if (strstr(test_cases[0].extra_data, expected_values[i]) != NULL) {
                    found_count++;
                }
            }
            
            printf("   ✓ Tìm thấy %d/%lu giá trị kỳ vọng trong extra_data\n", 
                   found_count, sizeof(expected_values) / sizeof(expected_values[0]));
            
            // In một phần extra_data
            printf("   - Một phần extra_data:\n");
            int preview_length = strlen(test_cases[0].extra_data) < 100 ? 
                                 strlen(test_cases[0].extra_data) : 100;
            char preview[101];
            strncpy(preview, test_cases[0].extra_data, preview_length);
            preview[preview_length] = '\0';
            printf("%s...\n", preview);
        } else {
            printf("   ✗ extra_data không được trích xuất\n");
        }
        
        // Giải phóng bộ nhớ
        free_test_cases(test_cases, count);
    } else {
        printf("   ✗ Không thể phân tích JSON với extra_data\n");
    }
    
    printf("=> Kiểm tra xử lý extra_data hoàn tất.\n");
}

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
            
            // Kiểm tra các trường bổ sung để xác nhận tích hợp hoàn chỉnh
            if (strcmp(test_cases[0].description, "Testing integration") == 0 &&
                strcmp(test_cases[0].target, "localhost") == 0 &&
                test_cases[0].timeout == 1000 &&
                test_cases[0].enabled == true) {
                printf("   ✓ Các trường bổ sung của test case được trích xuất chính xác\n");
            } else {
                printf("   ✗ Các trường bổ sung của test case không được trích xuất chính xác\n");
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
                log_message(LOG_LVL_DEBUG, "Test case %d: ID=%s, Name=%s, Desc=%s, Target=%s, Timeout=%d, Enabled=%s",
                           i+1, test_cases[i].id, test_cases[i].name, test_cases[i].description,
                           test_cases[i].target, test_cases[i].timeout,
                           test_cases[i].enabled ? "true" : "false");
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
            
            // Kiểm tra xem log có chứa thông tin về các trường mới được trích xuất không
            if (strstr(log_content, "Desc=") && strstr(log_content, "Target=") && 
                strstr(log_content, "Timeout=") && strstr(log_content, "Enabled=")) {
                printf("   ✓ Log chứa thông tin về các trường mới được trích xuất\n");
            } else {
                printf("   ✗ Log không chứa thông tin về các trường mới được trích xuất\n");
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
    printf("Ngày giờ: %s\n", "2025-03-20 03:10:52");
    printf("Người dùng: %s\n", "juno-kyojin");
    printf("=================================================\n");
    
    // Thiết lập môi trường kiểm thử
    setup_test_environment();
    
    // Chạy các bài kiểm thử cơ bản
    test_read_json_test_cases();
    test_parse_json_content();
    test_test_cases_to_json();
    test_error_handling();
    test_integration();
    
    // Chạy các bài kiểm thử mới
    test_default_values();
    test_extra_data();
    
    // Dọn dẹp môi trường kiểm thử
    cleanup_test_environment();
    
    printf("\n=================================================\n");
    printf("      HOÀN TẤT KIỂM THỬ PARSER_DATA.C\n");
    printf("=================================================\n");
    
    return 0;
}