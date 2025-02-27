#ifndef PACKET_PROCESS_H
#define PACKET_PROCESS_H

#include "parser_data.h"

// Đọc test case từ FIFO queue và thực thi
void process_queued_test_cases();

// Xử lý test case & lưu kết quả vào file
void execute_test_case(const test_case_t *test_case);

#endif // PACKET_PROCESS_H
