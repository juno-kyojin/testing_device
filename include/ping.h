#ifndef PING_H
#define PING_H

#include "types.h"
#include "cjson/cJSON.h"
#include <time.h>

// Ping command result
typedef struct {
    int packets_received;
    int packets_transmitted;
    float avg_time;
    float packet_loss;      // Packet loss percentage
    char status[16];        // "pass", "partial", or "fail"
} PingResult;

// Separated functions
int parse_host(const char *filepath, int index, char *host, size_t host_size, cJSON *result_json);
int ping(const char *host, PingResult *result);
void log_result(const PingResult *result, time_t start_time, time_t end_time);

void execute_ping(TestCase *test_case, const char *filepath, int index, cJSON *result_array);

#endif