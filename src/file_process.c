#include "file_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Mở một file với chế độ chỉ định.
 * @param file_name Tên file.
 * @param mode Chế độ mở file ("r", "w", "a"...).
 * @return Con trỏ FILE, hoặc NULL nếu lỗi.
 */
FILE *open_file(const char *file_name, const char *mode) {
    FILE *file = fopen(file_name, mode);
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s with mode %s\n", file_name, mode);
        perror("Open file failed");
    }
    return file;
}

/**
 * @brief Đọc toàn bộ nội dung file vào chuỗi động.
 * @param file_name Tên file cần đọc.
 * @return Chuỗi chứa nội dung file, cần free() sau khi sử dụng.
 */
char *read_file(const char *file_name) {
    FILE *file = open_file(file_name, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char *content = malloc(filesize + 1);
    if (!content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    fread(content, sizeof(char), filesize, file);
    content[filesize] = '\0';

    fclose(file);  // Đóng file sau khi đọc xong
    return content;
}

/**
 * @brief Ghi nội dung vào file.
 * @param file_name Tên file.
 * @param content Nội dung cần ghi.
 */
void write_to_file(const char *file_name, const char *content) {
    FILE *file = open_file(file_name, "a");
    if (!file) return;

    fprintf(file, "%s\n", content);
    fclose(file);
}

/**
 * @brief Ghi thời gian hiện tại vào file (log hệ thống).
 * @param file_name Tên file cần ghi.
 */
void printf_time_to_file(const char *file_name) {
    FILE *file = open_file(file_name, "a");
    if (!file) return;

    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (local != NULL) {
        fprintf(file, "TIME: %02d:%02d:%02d DATE:%02d-%02d-%04d\n",
                local->tm_hour, local->tm_min, local->tm_sec,
                local->tm_mday, local->tm_mon + 1, local->tm_year + 1900);
    } else {
        fprintf(file, "TIME: UNKNOWN DATE: UNKNOWN\n");
    }

    fclose(file);
}

/**
 * @brief Ghi thời gian vào file với xử lý lỗi nếu không lấy được thời gian.
 * @param file Con trỏ FILE đã mở sẵn.
 */
void printf_time_to_file_custom(FILE *file) {
    if (!file) return;

    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (local != NULL) {
        fprintf(file, "TIME: %02d:%02d:%02d DATE:%02d-%02d-%04d\n",
                local->tm_hour, local->tm_min, local->tm_sec,
                local->tm_mday, local->tm_mon + 1, local->tm_year + 1900);
    } else {
        fprintf(file, "TIME: UNKNOWN DATE: UNKNOWN\n");
    }
}

/**
 * @brief Xóa nội dung của file bằng cách mở file ở chế độ ghi rỗng.
 * @param file_name Tên file cần xóa nội dung.
 */
void clear_file_to_run(const char *file_name) {
    FILE *file = open_file(file_name, "w");
    if (!file) return;

    fclose(file);
    printf("File %s has been cleared.\n", file_name);
}
