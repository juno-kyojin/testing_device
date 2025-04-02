// #include "parser_option.h"
// #include "log.h"
// #include "file_process.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <getopt.h>
// #include <unistd.h>

// #define VERSION "1.0.0"

// /**
//  * @brief Thiết lập các giá trị mặc định cho tùy chọn dòng lệnh
//  * 
//  * @param options Con trỏ đến biến lưu các tùy chọn
//  */
// void set_default_options(cmd_options_t *options) {
//     if (!options) return;
    
//     strncpy(options->config_file, "config/config.json", sizeof(options->config_file) - 1);
//     options->config_file[sizeof(options->config_file) - 1] = '\0';
    
//     strncpy(options->log_file, "logs/testing_device.log", sizeof(options->log_file) - 1);
//     options->log_file[sizeof(options->log_file) - 1] = '\0';
    
//     strncpy(options->output_directory, "results", sizeof(options->output_directory) - 1);
//     options->output_directory[sizeof(options->output_directory) - 1] = '\0';
    
//     options->log_level = LOG_LVL_WARN;
//     options->verbose = false;
//     options->execute_all = false;
//     options->show_help = false;
//     options->show_version = false;
// }

// /**
//  * @brief Hiển thị thông tin trợ giúp
//  * 
//  * @param program_name Tên chương trình
//  */
// void show_usage(const char *program_name) {
//     printf("Sử dụng: %s [TÙY CHỌN]\n", program_name);
//     printf("Công cụ kiểm tra thiết bị mạng\n\n");
//     printf("Các tùy chọn:\n");
//     printf("  -c, --config=FILE       Chỉ định file cấu hình (mặc định: config/config.json)\n");
//     printf("  -l, --log=FILE          Ghi log vào file (mặc định: logs/testing_device.log)\n");
//     printf("  -o, --output=DIR        Thư mục lưu kết quả đầu ra (mặc định: results)\n");
//     printf("  -L, --log-level=LEVEL   Mức độ ghi log (0-3, 0: none, 3: debug)\n");
//     printf("  -a, --all               Thực thi tất cả các test case\n");
//     printf("  -v, --verbose           Hiển thị thông tin chi tiết\n");
//     printf("  -h, --help              Hiển thị thông tin trợ giúp này\n");
//     printf("  -V, --version           Hiển thị thông tin phiên bản\n");
//     printf("\nVí dụ:\n");
//     printf("  %s --config=config/multi_test_config.json\n", program_name);
//     printf("  %s -c config/speedtest_config.json -a\n", program_name);
// }

// /**
//  * @brief Hiển thị thông tin phiên bản
//  */
// void show_version(void) {
//     printf("Testing Device version %s\n", VERSION);
// }

// /**
//  * @brief In ra các tùy chọn hiện tại
//  * 
//  * @param options Con trỏ đến biến lưu các tùy chọn
//  */
// void print_options(const cmd_options_t *options) {
//     if (!options) return;
    
//     printf("Tùy chọn hiện tại:\n");
//     printf("  Config file: %s\n", options->config_file);
//     printf("  Log file: %s\n", options->log_file);
//     printf("  Output directory: %s\n", options->output_directory);
//     printf("  Log level: %d\n", options->log_level);
//     printf("  Execute all: %s\n", options->execute_all ? "Yes" : "No");
//     printf("  Verbose: %s\n", options->verbose ? "Yes" : "No");
// }

// /**
//  * @brief Phân tích các tham số dòng lệnh
//  * 
//  * @param argc Số lượng tham số
//  * @param argv Mảng tham số
//  * @param options Con trỏ đến biến lưu các tùy chọn
//  * @return int 0 nếu thành công, khác 0 nếu thất bại
//  */
// int parse_command_line(int argc, char *argv[], cmd_options_t *options) {
//     if (!options) return -1;
    
//     // Thiết lập giá trị mặc định
//     set_default_options(options);
    
//     static struct option long_options[] = {
//         {"config", required_argument, 0, 'c'},
//         {"log", required_argument, 0, 'l'},
//         {"output", required_argument, 0, 'o'},
//         {"log-level", required_argument, 0, 'L'},
//         {"all", no_argument, 0, 'a'},
//         {"verbose", no_argument, 0, 'v'},
//         {"help", no_argument, 0, 'h'},
//         {"version", no_argument, 0, 'V'},
//         {0, 0, 0, 0}
//     };
    
//     int opt;
//     int option_index = 0;
    
//     // Phân tích các tham số dòng lệnh
//     while ((opt = getopt_long(argc, argv, "c:l:o:L:avhV", long_options, &option_index)) != -1) {
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
//             case 'L':
//                 options->log_level = atoi(optarg);
//                 if (options->log_level < 0) options->log_level = 0;
//                 if (options->log_level > 3) options->log_level = 3;
//                 break;
//             case 'a':
//                 options->execute_all = true;
//                 break;
//             case 'v':
//                 options->verbose = true;
//                 break;
//             case 'h':
//                 options->show_help = true;
//                 break;
//             case 'V':
//                 options->show_version = true;
//                 break;
//             case '?':
//                 // getopt_long đã in thông báo lỗi
//                 return -1;
//             default:
//                 fprintf(stderr, "Tùy chọn không được hỗ trợ: %c\n", opt);
//                 return -1;
//         }
//     }
    
//     // Xử lý tham số không có flag (để tương thích với cách cũ)
//     if (optind < argc) {
//         strncpy(options->config_file, argv[optind], sizeof(options->config_file) - 1);
//         options->config_file[sizeof(options->config_file) - 1] = '\0';
//     }

//     // Kiểm tra biến môi trường CONFIG_FILE
//     const char* env_config = getenv("CONFIG_FILE");
//     if (env_config && strcmp(options->config_file, "config/config.json") == 0) {
//         strncpy(options->config_file, env_config, sizeof(options->config_file) - 1);
//         options->config_file[sizeof(options->config_file) - 1] = '\0';
//     }
    
//     // Tạo thư mục log và output nếu chưa tồn tại
//     char *log_dir = strdup(options->log_file);
//     if (log_dir) {
//         char *last_slash = strrchr(log_dir, '/');
//         if (last_slash) {
//             *last_slash = '\0';
//             create_directory(log_dir);
//         }
//         free(log_dir);
//     }
    
//     create_directory(options->output_directory);
    
//     return 0;
// }
