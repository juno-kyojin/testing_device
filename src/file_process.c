#include "file_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

FILE* open_file(const char* filename, file_mode_t mode, int max_retries) {
    const char* mode_str;
    switch (mode) {
        case FILE_MODE_READ:
            mode_str = "r";
            break;
        case FILE_MODE_WRITE:
            mode_str = "w";
            break;
        case FILE_MODE_APPEND:
            mode_str = "a";
            break;
        case FILE_MODE_READ_WRITE:
            mode_str = "r+";
            break;
        default:
            return NULL;
    }

    FILE* fp = NULL;
    int retries = 0;
    while (retries < max_retries) {
        fp = fopen(filename, mode_str);
        if (fp != NULL) {
            return fp;
        }
        retries++;
        sleep(1); // Đợi 1 giây trước khi thử lại
    }

    fprintf(stderr, "Failed to open file %s after %d retries: %s\n", filename, max_retries, strerror(errno));
    return NULL;
}

int read_file(const char* filename, char** buffer, size_t* size) {
    FILE* fp = open_file(filename, FILE_MODE_READ, 3);
    if (!fp) {
        return -1;
    }

    // Lấy kích thước file
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Cấp phát buffer
    *buffer = (char*)malloc(*size + 1);
    if (!*buffer) {
        fclose(fp);
        return -1;
    }

    // Đọc dữ liệu
    size_t bytes_read = fread(*buffer, 1, *size, fp);
    (*buffer)[bytes_read] = '\0';
    fclose(fp);

    if (bytes_read != *size) {
        free(*buffer);
        *buffer = NULL;
        return -1;
    }

    return 0;
}

ssize_t write_to_file(const char* filename, const void* data, size_t data_size, bool append) {
    file_mode_t mode = append ? FILE_MODE_APPEND : FILE_MODE_WRITE;
    FILE* fp = open_file(filename, mode, 3);
    if (!fp) {
        return -1;
    }

    size_t bytes_written = fwrite(data, 1, data_size, fp);
    if (bytes_written != data_size) {
        fclose(fp);
        return -1;
    }

    // Thêm ký tự xuống dòng
    fwrite("\n", 1, 1, fp);
    fflush(fp);
    fclose(fp);
    return (ssize_t)bytes_written;
}