#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_process.h"

#define TEST_FILE "test_file.txt"
#define MAX_RETRIES 3

// Hàm kiểm tra mở file thành công
void test_open_file_success() {
    printf("Testing open_file (success)... ");
    FILE *fp = open_file(TEST_FILE, FILE_MODE_WRITE, MAX_RETRIES);
    if (fp) {
        fprintf(fp, "Initial test data");
        fclose(fp);
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
}

// Hàm kiểm tra mở file thất bại
void test_open_file_failure() {
    printf("Testing open_file (failure with invalid path)... ");
    FILE *fp = open_file("/nonexistent/path/test.txt", FILE_MODE_WRITE, MAX_RETRIES);
    if (fp == NULL) {
        printf("PASSED\n");
    } else {
        fclose(fp);
        printf("FAILED\n");
    }
}

// Hàm kiểm tra ghi file thành công
void test_write_to_file_success() {
    printf("Testing write_to_file (success)... ");
    const char *data = "Hello, World!";
    ssize_t bytes_written = write_to_file(TEST_FILE, data, strlen(data), true);
    if (bytes_written == (ssize_t)strlen(data)) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
}

// Hàm kiểm tra ghi file thất bại
void test_write_to_file_failure() {
    printf("Testing write_to_file (failure with no permission)... ");
    const char *data = "Restricted data";
    ssize_t bytes_written = write_to_file("/root/restricted.txt", data, strlen(data), true);
    if (bytes_written == -1) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
}

// Hàm kiểm tra đọc file thành công
void test_read_file_success() {
    printf("Testing read_file (success)... ");
    char *buffer = NULL;
    size_t size;
    if (read_file(TEST_FILE, &buffer, &size) == 0 && buffer) {
        printf("PASSED, Read: %s, Size: %zu\n", buffer, size);
        free(buffer);
    } else {
        printf("FAILED\n");
        free(buffer);
    }
}

// Hàm kiểm tra đọc file thất bại
void test_read_file_failure() {
    printf("Testing read_file (failure with nonexistent file)... ");
    char *buffer = NULL;
    size_t size;
    if (read_file("/nonexistent/path/test.txt", &buffer, &size) == -1 && buffer == NULL) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
        free(buffer);
    }
}

int main() {
    // Chạy các test case
    test_open_file_success();
    test_open_file_failure();
    test_write_to_file_success();
    test_write_to_file_failure();
    test_read_file_success();
    test_read_file_failure();

    // Xóa file test sau khi hoàn thành
    remove(TEST_FILE);
    return 0;
}