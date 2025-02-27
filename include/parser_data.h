#ifndef PARSER_DATA_H
#define PARSER_DATA_H

#include <stdbool.h>
#include <stdio.h>

#define MAX_LENGTH 256

typedef struct {
    char id[MAX_LENGTH];
    char target[MAX_LENGTH];
    char protocol[10];
    int port;
} test_case_t;

// Đọc test case từ file JSON
test_case_t *read_json_test_cases(const char *filename, int *count);

// Lọc test case kiểm thử mà thiết bị có thể chạy
test_case_t *filter_valid_test_cases(test_case_t *test_cases, int count, int *valid_count);

// Đưa test case vào hàng đợi FIFO
void enqueue_test_cases_to_fifo(test_case_t *test_cases, int count);

#endif // PARSER_DATA_H
