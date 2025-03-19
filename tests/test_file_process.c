/**
 * @file test_file_process.c
 * @brief Kiểm thử các chức năng file_process
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <assert.h>
 #include "include/file_process.h"
 
 // Đường dẫn file test để tránh ảnh hưởng đến file thật
 #define TEST_FILE "test_file_process.txt"
 #define TEST_FILE_COPY "test_file_process_copy.txt"
 #define TEST_DIR "test_file_process_dir"
 #define TEST_FILE_IN_DIR "test_file_process_dir/test_file.txt"
 
 /**
  * @brief Kiểm tra tạo, đọc, ghi và xóa file
  */
 void test_basic_file_operations() {
     printf("\n--- Kiểm tra tạo, đọc, ghi và xóa file ---\n");
     const char *content = "Hello, this is a test content for file operations!\n";
     size_t content_len = strlen(content);
     char *read_buffer = NULL;
     size_t read_size = 0;
     
     // Kiểm tra ghi file
     printf("1. Ghi file '%s'...\n", TEST_FILE);
     int ret = write_file(TEST_FILE, content, content_len);
     if (ret == 0) {
         printf("   ✓ Ghi file thành công\n");
     } else {
         printf("   ✗ Ghi file thất bại\n");
         return;
     }
     
     // Kiểm tra file tồn tại
     printf("2. Kiểm tra file tồn tại...\n");
     if (file_exists(TEST_FILE)) {
         printf("   ✓ File tồn tại\n");
     } else {
         printf("   ✗ File không tồn tại\n");
         return;
     }
     
     // Kiểm tra đọc file
     printf("3. Đọc nội dung file...\n");
     ret = read_file(TEST_FILE, &read_buffer, &read_size);
     if (ret == 0 && read_buffer != NULL && read_size == content_len) {
         printf("   ✓ Đọc file thành công (%zu bytes)\n", read_size);
         if (strcmp(read_buffer, content) == 0) {
             printf("   ✓ Nội dung đọc được chính xác\n");
         } else {
             printf("   ✗ Nội dung đọc được không khớp\n");
             free(read_buffer);
             return;
         }
     } else {
         printf("   ✗ Đọc file thất bại\n");
         if (read_buffer != NULL) free(read_buffer);
         return;
     }
     
     // Giải phóng bộ nhớ
     if (read_buffer != NULL) {
         free(read_buffer);
         read_buffer = NULL;
     }
     
     // Kiểm tra kích thước file
     printf("4. Kiểm tra kích thước file...\n");
     long file_size = get_file_size(TEST_FILE);
     if (file_size == content_len) {
         printf("   ✓ Kích thước file chính xác (%ld bytes)\n", file_size);
     } else {
         printf("   ✗ Kích thước file không chính xác (kỳ vọng: %zu, thực tế: %ld)\n", 
                content_len, file_size);
         return;
     }
     
     // Kiểm tra thời gian sửa đổi
     printf("5. Kiểm tra thời gian sửa đổi file...\n");
     time_t mod_time = get_file_modification_time(TEST_FILE);
     if (mod_time > 0) {
         printf("   ✓ Lấy thời gian sửa đổi thành công (%ld)\n", mod_time);
     } else {
         printf("   ✗ Lấy thời gian sửa đổi thất bại\n");
         return;
     }
     
     // Kiểm tra xóa file
     printf("6. Xóa file...\n");
     ret = delete_file(TEST_FILE);
     if (ret == 0) {
         printf("   ✓ Xóa file thành công\n");
         if (!file_exists(TEST_FILE)) {
             printf("   ✓ File không còn tồn tại\n");
         } else {
             printf("   ✗ File vẫn còn tồn tại\n");
             return;
         }
     } else {
         printf("   ✗ Xóa file thất bại\n");
         return;
     }
     
     printf("=> Kiểm tra cơ bản hoàn tất: TẤT CẢ OK.\n");
 }
 
 /**
  * @brief Kiểm tra thêm nội dung vào file
  */
 void test_append_file() {
     printf("\n--- Kiểm tra thêm nội dung vào file ---\n");
     const char *content1 = "Nội dung ban đầu\n";
     const char *content2 = "Nội dung thêm vào\n";
     char *read_buffer = NULL;
     size_t read_size = 0;
     
     // Tạo file ban đầu
     printf("1. Tạo file ban đầu với nội dung...\n");
     int ret = write_file(TEST_FILE, content1, strlen(content1));
     if (ret != 0) {
         printf("   ✗ Ghi file ban đầu thất bại\n");
         return;
     }
     
     // Thêm nội dung vào file
     printf("2. Thêm nội dung vào file...\n");
     ret = append_to_file(TEST_FILE, content2, strlen(content2));
     if (ret == 0) {
         printf("   ✓ Thêm nội dung thành công\n");
     } else {
         printf("   ✗ Thêm nội dung thất bại\n");
         delete_file(TEST_FILE);
         return;
     }
     
     // Kiểm tra nội dung sau khi thêm
     printf("3. Kiểm tra nội dung sau khi thêm...\n");
     ret = read_file(TEST_FILE, &read_buffer, &read_size);
     if (ret == 0 && read_buffer != NULL) {
         size_t expected_size = strlen(content1) + strlen(content2);
         if (read_size == expected_size) {
             printf("   ✓ Kích thước nội dung chính xác (%zu bytes)\n", read_size);
             char *expected_content = (char*)malloc(expected_size + 1);
             if (expected_content) {
                 strcpy(expected_content, content1);
                 strcat(expected_content, content2);
                 
                 if (strcmp(read_buffer, expected_content) == 0) {
                     printf("   ✓ Nội dung sau khi thêm chính xác\n");
                 } else {
                     printf("   ✗ Nội dung sau khi thêm không khớp\n");
                 }
                 
                 free(expected_content);
             }
         } else {
             printf("   ✗ Kích thước nội dung không chính xác (kỳ vọng: %zu, thực tế: %zu)\n",
                    strlen(content1) + strlen(content2), read_size);
         }
         
         free(read_buffer);
     } else {
         printf("   ✗ Đọc file sau khi thêm nội dung thất bại\n");
     }
     
     // Dọn dẹp
     delete_file(TEST_FILE);
     printf("=> Kiểm tra thêm nội dung hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra sao chép file
  */
 void test_copy_file() {
     printf("\n--- Kiểm tra sao chép file ---\n");
     const char *content = "Nội dung để kiểm tra sao chép file\n";
     char *read_buffer = NULL;
     size_t read_size = 0;
     
     // Tạo file nguồn
     printf("1. Tạo file nguồn '%s'...\n", TEST_FILE);
     int ret = write_file(TEST_FILE, content, strlen(content));
     if (ret != 0) {
         printf("   ✗ Tạo file nguồn thất bại\n");
         return;
     }
     
     // Sao chép file
     printf("2. Sao chép file sang '%s'...\n", TEST_FILE_COPY);
     ret = copy_file(TEST_FILE, TEST_FILE_COPY);
     if (ret == 0) {
         printf("   ✓ Sao chép file thành công\n");
     } else {
         printf("   ✗ Sao chép file thất bại\n");
         delete_file(TEST_FILE);
         return;
     }
     
     // Kiểm tra nội dung file sau khi sao chép
     printf("3. Kiểm tra nội dung file sau khi sao chép...\n");
     ret = read_file(TEST_FILE_COPY, &read_buffer, &read_size);
     if (ret == 0 && read_buffer != NULL) {
         if (read_size == strlen(content)) {
             printf("   ✓ Kích thước chính xác (%zu bytes)\n", read_size);
             if (strcmp(read_buffer, content) == 0) {
                 printf("   ✓ Nội dung file sao chép chính xác\n");
             } else {
                 printf("   ✗ Nội dung file sao chép không khớp\n");
             }
         } else {
             printf("   ✗ Kích thước không chính xác (kỳ vọng: %zu, thực tế: %zu)\n",
                    strlen(content), read_size);
         }
         
         free(read_buffer);
     } else {
         printf("   ✗ Đọc file sao chép thất bại\n");
     }
     
     // Dọn dẹp
     delete_file(TEST_FILE);
     delete_file(TEST_FILE_COPY);
     printf("=> Kiểm tra sao chép file hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra thao tác với thư mục
  */
 void test_directory_operations() {
     printf("\n--- Kiểm tra thao tác với thư mục ---\n");
     const char *content = "File trong thư mục test\n";
     
     // Tạo thư mục
     printf("1. Tạo thư mục '%s'...\n", TEST_DIR);
     int ret = create_directory(TEST_DIR);
     if (ret == 0) {
         printf("   ✓ Tạo thư mục thành công\n");
     } else {
         printf("   ✗ Tạo thư mục thất bại\n");
         return;
     }
     
     // Tạo file trong thư mục
     printf("2. Tạo file trong thư mục...\n");
     ret = write_file(TEST_FILE_IN_DIR, content, strlen(content));
     if (ret == 0) {
         printf("   ✓ Tạo file trong thư mục thành công\n");
     } else {
         printf("   ✗ Tạo file trong thư mục thất bại\n");
         // Trong thế giới thực, bạn sẽ cần một hàm để xóa thư mục
         return;
     }
     
     // Kiểm tra file tồn tại trong thư mục
     printf("3. Kiểm tra file tồn tại trong thư mục...\n");
     if (file_exists(TEST_FILE_IN_DIR)) {
         printf("   ✓ File trong thư mục tồn tại\n");
     } else {
         printf("   ✗ File trong thư mục không tồn tại\n");
         return;
     }
     
     // Đọc file từ thư mục
     printf("4. Đọc file từ thư mục...\n");
     char *read_buffer = NULL;
     size_t read_size = 0;
     ret = read_file(TEST_FILE_IN_DIR, &read_buffer, &read_size);
     if (ret == 0 && read_buffer != NULL) {
         if (strcmp(read_buffer, content) == 0) {
             printf("   ✓ Nội dung file trong thư mục chính xác\n");
         } else {
             printf("   ✗ Nội dung file trong thư mục không khớp\n");
         }
         free(read_buffer);
     } else {
         printf("   ✗ Đọc file từ thư mục thất bại\n");
     }
     
     // Dọn dẹp
     printf("5. Dọn dẹp thư mục test...\n");
     delete_file(TEST_FILE_IN_DIR);
     // Lưu ý: file_process.c không có hàm xóa thư mục
     // Bạn có thể sẽ cần một hàm rmdir() trong ứng dụng thực tế
     printf("   ✓ Đã xóa file test trong thư mục\n");
     
     printf("=> Kiểm tra thao tác với thư mục hoàn tất.\n");
 }
 
 /**
  * @brief Kiểm tra tạo file tạm
  */
 void test_temp_file() {
     printf("\n--- Kiểm tra tạo file tạm ---\n");
     char temp_path[256];
     const char *prefix = "testfile";
     const char *content = "Nội dung file tạm\n";
     
     // Tạo file tạm
     printf("1. Tạo file tạm với tiền tố '%s'...\n", prefix);
     char *path = create_temp_file(prefix, temp_path, sizeof(temp_path));
     if (path != NULL) {
         printf("   ✓ Tạo file tạm thành công: %s\n", temp_path);
     } else {
         printf("   ✗ Tạo file tạm thất bại\n");
         return;
     }
     
     // Ghi nội dung
         // Ghi nội dung vào file tạm
    printf("2. Ghi nội dung vào file tạm...\n");
    int ret = write_file(temp_path, content, strlen(content));
    if (ret == 0) {
        printf("   ✓ Ghi nội dung vào file tạm thành công\n");
    } else {
        printf("   ✗ Ghi nội dung vào file tạm thất bại\n");
        delete_file(temp_path);
        return;
    }
    
    // Kiểm tra đọc file tạm
    printf("3. Đọc nội dung từ file tạm...\n");
    char *read_buffer = NULL;
    size_t read_size = 0;
    ret = read_file(temp_path, &read_buffer, &read_size);
    if (ret == 0 && read_buffer != NULL) {
        if (strcmp(read_buffer, content) == 0) {
            printf("   ✓ Nội dung file tạm chính xác\n");
        } else {
            printf("   ✗ Nội dung file tạm không khớp\n");
        }
        free(read_buffer);
    } else {
        printf("   ✗ Đọc nội dung file tạm thất bại\n");
    }
    
    // Xóa file tạm
    printf("4. Xóa file tạm...\n");
    ret = delete_file(temp_path);
    if (ret == 0) {
        printf("   ✓ Xóa file tạm thành công\n");
    } else {
        printf("   ✗ Xóa file tạm thất bại\n");
    }
    
    printf("=> Kiểm tra file tạm hoàn tất.\n");
}

/**
 * @brief Kiểm tra đọc một phần của file
 */
void test_read_file_chunk() {
    printf("\n--- Kiểm tra đọc một phần của file ---\n");
    const char *content = "Dòng 1: Đây là nội dung cho test đọc từng phần\nDòng 2: Tiếp tục nội dung\nDòng 3: Kết thúc nội dung test\n";
    char buffer[64] = {0};
    size_t bytes_read = 0;
    
    // Tạo file test
    printf("1. Tạo file test với nội dung nhiều dòng...\n");
    int ret = write_file(TEST_FILE, content, strlen(content));
    if (ret != 0) {
        printf("   ✗ Tạo file test thất bại\n");
        return;
    }
    
    // Đọc phần đầu file
    printf("2. Đọc 20 bytes đầu tiên của file...\n");
    ret = read_file_chunk(TEST_FILE, buffer, 20, 0, &bytes_read);
    if (ret == 0) {
        buffer[bytes_read] = '\0'; // Đảm bảo kết thúc chuỗi
        printf("   ✓ Đọc thành công %zu bytes\n", bytes_read);
        printf("   Nội dung: '%s'\n", buffer);
        
        if (strncmp(buffer, content, 20) == 0) {
            printf("   ✓ Nội dung chính xác\n");
        } else {
            printf("   ✗ Nội dung không khớp\n");
        }
    } else {
        printf("   ✗ Đọc phần đầu file thất bại\n");
        delete_file(TEST_FILE);
        return;
    }
    
    // Đọc phần giữa file
    printf("3. Đọc 20 bytes từ vị trí 30 của file...\n");
    memset(buffer, 0, sizeof(buffer));
    ret = read_file_chunk(TEST_FILE, buffer, 20, 30, &bytes_read);
    if (ret == 0) {
        buffer[bytes_read] = '\0';
        printf("   ✓ Đọc thành công %zu bytes\n", bytes_read);
        printf("   Nội dung: '%s'\n", buffer);
        
        if (strncmp(buffer, content + 30, 20) == 0) {
            printf("   ✓ Nội dung chính xác\n");
        } else {
            printf("   ✗ Nội dung không khớp\n");
        }
    } else {
        printf("   ✗ Đọc phần giữa file thất bại\n");
    }
    
    // Đọc quá kích thước file
    printf("4. Đọc từ vị trí vượt quá kích thước file...\n");
    long file_size = get_file_size(TEST_FILE);
    if (file_size > 0) {
        memset(buffer, 0, sizeof(buffer));
        ret = read_file_chunk(TEST_FILE, buffer, 20, file_size + 10, &bytes_read);
        if (ret == 0) {
            printf("   ✓ Hàm xử lý đúng\n");
            printf("   Đọc được %zu bytes (kỳ vọng: 0)\n", bytes_read);
            if (bytes_read == 0) {
                printf("   ✓ Không đọc được dữ liệu khi vượt quá kích thước file\n");
            } else {
                printf("   ✗ Đọc được dữ liệu dù vượt quá kích thước file\n");
            }
        } else {
            printf("   ✓ Hàm báo lỗi khi đọc vượt quá kích thước file (điều này cũng có thể chấp nhận được)\n");
        }
    }
    
    // Dọn dẹp
    delete_file(TEST_FILE);
    printf("=> Kiểm tra đọc từng phần của file hoàn tất.\n");
}

/**
 * @brief Kiểm tra xử lý lỗi
 */
void test_error_handling() {
    printf("\n--- Kiểm tra xử lý lỗi ---\n");
    
    // Thử đọc file không tồn tại
    printf("1. Đọc file không tồn tại...\n");
    char *buffer = NULL;
    size_t size = 0;
    int ret = read_file("file_khong_ton_tai.txt", &buffer, &size);
    if (ret != 0) {
        printf("   ✓ Xử lý đúng khi đọc file không tồn tại\n");
    } else {
        printf("   ✗ Hàm đọc file đã thành công dù file không tồn tại\n");
        if (buffer) free(buffer);
    }
    
    // Thử xóa file không tồn tại
    printf("2. Xóa file không tồn tại...\n");
    ret = delete_file("file_khong_ton_tai.txt");
    if (ret != 0) {
        printf("   ✓ Xử lý đúng khi xóa file không tồn tại\n");
    } else {
        printf("   ✗ Hàm xóa file đã thành công dù file không tồn tại\n");
    }
    
    // Thử ghi file vào thư mục không tồn tại
    printf("3. Ghi file vào thư mục không tồn tại...\n");
    ret = write_file("thu_muc_khong_ton_tai/file.txt", "test", 4);
    if (ret != 0) {
        printf("   ✓ Xử lý đúng khi ghi file vào thư mục không tồn tại\n");
    } else {
        printf("   ✗ Hàm ghi file đã thành công dù thư mục không tồn tại\n");
    }
    
    printf("=> Kiểm tra xử lý lỗi hoàn tất.\n");
}

int main() {
    printf("\n=================================================\n");
    printf("      KIỂM THỬ MODULE FILE_PROCESS.C\n");
    printf("=================================================\n");
    
    // Chạy các test case
    test_basic_file_operations();
    test_append_file();
    test_copy_file();
    test_directory_operations();
    test_temp_file();
    test_read_file_chunk();
    test_error_handling();
    
    printf("\n=================================================\n");
    printf("      HOÀN TẤT KIỂM THỬ FILE_PROCESS.C\n");
    printf("=================================================\n");
    
    return 0;
}