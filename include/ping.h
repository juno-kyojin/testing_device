#ifndef PING_H
#define PING_H

#include "types.h"
#include "cjson/cJSON.h"
#include <time.h>
// Kết quả của lệnh ping
typedef struct {
    int packets_received;
    int packets_transmitted;
    float avg_time;
    char status[8]; // "pass" hoặc "fail"
} PingResult;

// Các hàm được tách biệt
int parse_host(const char *filepath, int index, char *host, size_t host_size, cJSON *result_json);
int ping(const char *host, PingResult *result);
void log_result(const PingResult *result, time_t start_time, time_t end_time);

void execute_ping(TestCase *test_case, const char *filepath, int index, cJSON *result_array);

#endif