#ifndef GET_DATA_H
#define GET_DATA_H

#include <stdbool.h>
#include <libssh/libssh.h>
#include "tc.h"  // Include tc.h to access the test_result_info_t definition

// Định nghĩa cấu trúc lưu thông tin kết nối
typedef struct {
    char host[256];
    int port;
    char username[64];
    char password[64];
    ssh_session session;
} connection_info_t;

// Định nghĩa cấu trúc lưu cấu hình mạng
typedef struct {
    char interface[16];
    char ip_address[16];
    char subnet_mask[16];
    char gateway[16];
    char dns_server[16];
    bool use_dhcp;
} network_config_t;

// Khai báo các hàm
connection_info_t* init_connection(const char* host, int port, const char* username, const char* password);
int receive_network_config_from_pc(connection_info_t* conn, network_config_t* config);
int receive_test_cases_from_pc(connection_info_t* conn, const char* output_file);
int send_progress_to_pc(connection_info_t* conn, int completed, int total, int success_count, int fail_count);
int send_results_to_pc(connection_info_t* conn, test_result_info_t* results, int count);
int send_logs_to_pc(connection_info_t* conn, const char* log_file);
int send_compressed_results_to_pc(connection_info_t* conn, const char* compressed_file);
int close_connection(connection_info_t* conn);

#endif // GET_DATA_H