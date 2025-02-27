#ifndef FILE_PROCESS_H
#define FILE_PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mở file với chế độ chỉ định
FILE *open_file(const char *file_name, const char *mode);

// Đọc toàn bộ nội dung file vào chuỗi động
char *read_file(const char *file_name);

// Ghi nội dung vào file
void write_to_file(const char *file_name, const char *content);

// Ghi thời gian vào file (dùng để log)
void printf_time_to_file(const char *file_name);
void printf_time_to_file_custom(FILE *file);

// Xóa nội dung file
void clear_file_to_run(const char *file_name);

#endif // FILE_PROCESS_H
