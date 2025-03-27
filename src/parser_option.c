// #include "parser_option.h"
// #include "log.h"
// #include "file_process.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <getopt.h>
// #include <unistd.h>
// #include "cjson/cJSON.h"

// // Ánh xạ chuỗi loại mạng sang enum
// static const struct {
//     const char *name;
//     network_type_t type;
// } network_type_map[] = {
//     { "lan", NETWORK_LAN },
//     { "wan", NETWORK_WAN },
//     { "both", NETWORK_BOTH },
//     { NULL, NETWORK_LAN }  // Kết thúc mảng
// };

// void set_default_options(cmd_options_t *options) {
//     if (!options) return;

//     strncpy(options->config_file, "config/config.json", sizeof(options->config_file) - 1);
//     options->config_file[sizeof(options->config_file) - 1] = '\0';

//     strncpy(options->log_file, "logs/testing_device.log", sizeof(options->log_file) - 1);
//     options->log_file[sizeof(options->log_file) - 1] = '\0';

//     strncpy(options->output_directory, "results", sizeof(options->output_directory) - 1);
//     options->output_directory[sizeof(options->output_directory) - 1] = '\0';

//     options->network_type = NETWORK_LAN;
//     options->log_level = LOG_LVL_WARN;
//     options->thread_count = 1;
//     options->daemon_mode = false;
//     options->verbose = false;
    
//     strncpy(options->connection_host, "localhost", sizeof(options->connection_host) - 1);
//     options->connection_host[sizeof(options->connection_host) - 1] = '\0';
    
//     options->connection_port = 22;
    
//     strncpy(options->username, "", sizeof(options->username) - 1);
//     options->username[sizeof(options->username) - 1] = '\0';
    
//     strncpy(options->password, "", sizeof(options->password) - 1);
//     options->password[sizeof(options->password) - 1] = '\0';
// }

// void show_usage(const char *program_name) {
//     printf("Sử dụng: %s [TÙY CHỌN]...\n", program_name);
//     printf("Công cụ kiểm tra mạng cho thiết bị.\n\n");
//     printf("Tùy chọn:\n");
//     printf("  -c, --config=FILE      Sử dụng file cấu hình\n");
//     printf("  -l, --log=FILE         Ghi log vào file\n");
//     printf("  -o, --output=DIR       Thư mục đầu ra cho kết quả\n");
//     printf("  -n, --network=TYPE     Loại mạng (lan, wan, both)\n");
//     printf("  -L, --log-level=LEVEL  Mức độ log (0-3, 0: none, 3: debug)\n");
//     printf("  -t, --threads=NUM      Số lượng thread (mặc định: 1)\n");
//     printf("  -d, --daemon           Chạy trong chế độ daemon\n");
//     printf("  -v, --verbose          Hiển thị thông tin chi tiết\n");
//     printf("  -h, --host=HOST        Địa chỉ máy chủ kết nối\n");
//     printf("  -p, --port=PORT        Cổng kết nối (mặc định: 22)\n");
//     printf("  -u, --username=USER    Tên đăng nhập\n");
//     printf("  -P, --password=PASS    Mật khẩu\n");
//     printf("      --help             Hiển thị thông tin trợ giúp này\n");
// }

// network_type_t string_to_network_type(const char *network_str) {
//     if (!network_str) return NETWORK_LAN;

//     for (int i = 0; network_type_map[i].name != NULL; i++) {
//         if (strcasecmp(network_str, network_type_map[i].name) == 0) {
//             return network_type_map[i].type;
//         }
//     }
//     return NETWORK_LAN;  // Mặc định
// }

// const char* network_type_to_string(network_type_t type) {
//     for (int i = 0; network_type_map[i].name != NULL; i++) {
//         if (type == network_type_map[i].type) {
//             return network_type_map[i].name;
//         }
//     }
//     return "lan";  // Mặc định
// }

// int parse_command_line(int argc, char *argv[], cmd_options_t *options) {
//     if (!options) return -1;

//     // Đặt giá trị mặc định trước
//     set_default_options(options);

//     static struct option long_options[] = {
//         {"config", required_argument, 0, 'c'},
//         {"log", required_argument, 0, 'l'},
//         {"output", required_argument, 0, 'o'},
//         {"network", required_argument, 0, 'n'},
//         {"log-level", required_argument, 0, 'L'},
//         {"threads", required_argument, 0, 't'},
//         {"daemon", no_argument, 0, 'd'},
//         {"verbose", no_argument, 0, 'v'},
//         {"host", required_argument, 0, 'h'},
//         {"port", required_argument, 0, 'p'},
//         {"username", required_argument, 0, 'u'},
//         {"password", required_argument, 0, 'P'},
//         {"help", no_argument, 0, 'H'},
//         {0, 0, 0, 0}
//     };

//     int opt;
//     int option_index = 0;
//     while ((opt = getopt_long(argc, argv, "c:l:o:n:L:t:dvh:p:u:P:", long_options, &option_index)) != -1) {
//         switch (opt) {
//             case 'c':
//                 strncpy(options->config_file, optarg, sizeof(options->config_file) - 1);
//                 options->config_file[sizeof(options->config_file) - 1] = '\0';
//                 break;
//             case 'l':
//                 strncpy(options->log_file, optarg, sizeof(options->log_file) - 1);
//                 options->log_file[sizeof(options->log_file) - 1] = '\0';
//                 break;
//             case 'o':
//                 strncpy(options->output_directory, optarg, sizeof(options->output_directory) - 1);
//                 options->output_directory[sizeof(options->output_directory) - 1] = '\0';
//                 break;
//             case 'n':
//                 options->network_type = string_to_network_type(optarg);
//                 break;
//             case 'L':
//                 options->log_level = atoi(optarg);
//                 if (options->log_level < 0) options->log_level = 0;
//                 if (options->log_level > 3) options->log_level = 3;
//                 break;
//             case 't':
//                 options->thread_count = atoi(optarg);
//                 if (options->thread_count < 1) options->thread_count = 1;
//                 break;
//             case 'd':
//                 options->daemon_mode = true;
//                 break;
//             case 'v':
//                 options->verbose = true;
//                 break;
//             case 'h':
//                 strncpy(options->connection_host, optarg, sizeof(options->connection_host) - 1);
//                 options->connection_host[sizeof(options->connection_host) - 1] = '\0';
//                 break;
//             case 'p':
//                 options->connection_port = atoi(optarg);
//                 if (options->connection_port <= 0 || options->connection_port > 65535)
//                     options->connection_port = 22;
//                 break;
//             case 'u':
//                 strncpy(options->username, optarg, sizeof(options->username) - 1);
//                 options->username[sizeof(options->username) - 1] = '\0';
//                 break;
//             case 'P':
//                 strncpy(options->password, optarg, sizeof(options->password) - 1);
//                 options->password[sizeof(options->password) - 1] = '\0';
//                 break;
//             case 'H':
//                 show_usage(argv[0]);
//                 return 1;  // Kết thúc chương trình sau khi hiện help
//             case '?':
//                 // getopt_long đã in thông báo lỗi
//                 return -1;
//             default:
//                 return -1;
//         }
//     }

//     // Đọc file cấu hình nếu tồn tại
//     if (access(options->config_file, F_OK) == 0) {
//         if (read_config_file(options->config_file, options) != 0) {
//             fprintf(stderr, "Lỗi khi đọc file cấu hình: %s\n", options->config_file);
//             return -1;
//         }
//     }

//     return validate_options(options);
// }

// int read_config_file(const char *config_file, cmd_options_t *options) {
//     if (!config_file || !options) return -1;

//     char *json_str = NULL;
//     size_t size;
    
//     if (read_file(config_file, &json_str, &size) != 0) {
//         return -1;
//     }

//     cJSON *root = cJSON_Parse(json_str);
//     free(json_str);

//     if (!root) {
//         return -1;
//     }

//     // Chỉ đọc các tùy chọn từ file nếu chưa được đặt trên dòng lệnh
//     cJSON *log_file = cJSON_GetObjectItem(root, "log_file");
//     if (log_file && cJSON_IsString(log_file)) {
//         strncpy(options->log_file, log_file->valuestring, sizeof(options->log_file) - 1);
//         options->log_file[sizeof(options->log_file) - 1] = '\0';
//     }

//     cJSON *output_dir = cJSON_GetObjectItem(root, "output_directory");
//     if (output_dir && cJSON_IsString(output_dir)) {
//         strncpy(options->output_directory, output_dir->valuestring, sizeof(options->output_directory) - 1);
//         options->output_directory[sizeof(options->output_directory) - 1] = '\0';
//     }

//     cJSON *network_type = cJSON_GetObjectItem(root, "network_type");
//     if (network_type && cJSON_IsString(network_type)) {
//         options->network_type = string_to_network_type(network_type->valuestring);
//     }

//     cJSON *log_level = cJSON_GetObjectItem(root, "log_level");
//     if (log_level && cJSON_IsNumber(log_level)) {
//         options->log_level = log_level->valueint;
//         if (options->log_level < 0) options->log_level = 0;
//         if (options->log_level > 3) options->log_level = 3;
//     }

//     cJSON *thread_count = cJSON_GetObjectItem(root, "thread_count");
//     if (thread_count && cJSON_IsNumber(thread_count)) {
//         options->thread_count = thread_count->valueint;
//         if (options->thread_count < 1) options->thread_count = 1;
//     }

//     cJSON *daemon_mode = cJSON_GetObjectItem(root, "daemon_mode");
//     if (daemon_mode && cJSON_IsBool(daemon_mode)) {
//         options->daemon_mode = cJSON_IsTrue(daemon_mode);
//     }

//     cJSON *verbose = cJSON_GetObjectItem(root, "verbose");
//     if (verbose && cJSON_IsBool(verbose)) {
//         options->verbose = cJSON_IsTrue(verbose);
//     }

//     cJSON *connection = cJSON_GetObjectItem(root, "connection");
//     if (connection && cJSON_IsObject(connection)) {
//         cJSON *host = cJSON_GetObjectItem(connection, "host");
//         if (host && cJSON_IsString(host)) {
//             strncpy(options->connection_host, host->valuestring, sizeof(options->connection_host) - 1);
//             options->connection_host[sizeof(options->connection_host) - 1] = '\0';
//         }

//         cJSON *port = cJSON_GetObjectItem(connection, "port");
//         if (port && cJSON_IsNumber(port)) {
//             options->connection_port = port->valueint;
//             if (options->connection_port <= 0 || options->connection_port > 65535)
//                 options->connection_port = 22;
//         }

//         cJSON *username = cJSON_GetObjectItem(connection, "username");
//         if (username && cJSON_IsString(username)) {
//             strncpy(options->username, username->valuestring, sizeof(options->username) - 1);
//             options->username[sizeof(options->username) - 1] = '\0';
//         }

//         cJSON *password = cJSON_GetObjectItem(connection, "password");
//         if (password && cJSON_IsString(password)) {
//             strncpy(options->password, password->valuestring, sizeof(options->password) - 1);
//             options->password[sizeof(options->password) - 1] = '\0';
//         }
//     }

//     cJSON_Delete(root);
//     return 0;
// }

// int validate_options(const cmd_options_t *options) {
//     if (!options) return -1;

//     // Kiểm tra các thư mục tồn tại hoặc có thể tạo
//     char *log_dir = strdup(options->log_file);
//     if (!log_dir) {
//         return -1;
//     }
    
//     char *last_slash = strrchr(log_dir, '/');
//     if (last_slash) {
//         *last_slash = '\0';
//         // Thử tạo thư mục nếu nó không tồn tại
//         if (access(log_dir, F_OK) != 0) {
//             char mkdir_cmd[512];
//             snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", log_dir);
//             int result = system(mkdir_cmd);
//             if (result != 0) {
//                 fprintf(stderr, "Không thể tạo thư mục log: %s\n", log_dir);
//                 free(log_dir);
//                 return -1;
//             }
//         }
//     }
//     free(log_dir);

//     // Kiểm tra thư mục đầu ra
//     if (access(options->output_directory, F_OK) != 0) {
//         char mkdir_cmd[512];
//         snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", options->output_directory);
//         int result = system(mkdir_cmd);
//         if (result != 0) {
//             fprintf(stderr, "Không thể tạo thư mục đầu ra: %s\n", options->output_directory);
//             return -1;
//         }
//     }

//     // Kiểm tra các giá trị khác nếu cần
//     if (options->thread_count < 1) {
//         fprintf(stderr, "Số lượng thread không hợp lệ: %d\n", options->thread_count);
//         return -1;
//     }

//     return 0;
// }

// void print_options(const cmd_options_t *options) {
//     if (!options) return;

//     printf("Tùy chọn hiện tại:\n");
//     printf("  Config file: %s\n", options->config_file);
//     printf("  Log file: %s\n", options->log_file);
//     printf("  Output directory: %s\n", options->output_directory);
//     printf("  Network type: %s\n", network_type_to_string(options->network_type));
//     printf("  Log level: %d\n", options->log_level);
//     printf("  Thread count: %d\n", options->thread_count);
//     printf("  Daemon mode: %s\n", options->daemon_mode ? "Yes" : "No");
//     printf("  Verbose: %s\n", options->verbose ? "Yes" : "No");
//     printf("  Connection:\n");
//     printf("    Host: %s\n", options->connection_host);
//     printf("    Port: %d\n", options->connection_port);
//     printf("    Username: %s\n", options->username);
//     printf("    Password: %s\n", options->password[0] ? "****" : "Not set");
// }
