/**
 * @file parser_option.h
 * @brief Xử lý tham số dòng lệnh và file cấu hình
 */

 #ifndef PARSER_OPTION_H
 #define PARSER_OPTION_H
 
 #include <stdbool.h>
 #include "parser_data.h"
 
 typedef struct {
     char config_file[256];
     char log_file[256];
     char output_directory[256];
     network_type_t network_type;
     int log_level;
     int thread_count;
     bool verbose;
 } cmd_options_t;
 
 int parse_command_line(int argc, char *argv[], cmd_options_t *options);
 void show_usage(const char *program_name);
 void set_default_options(cmd_options_t *options);
 int read_config_file(const char *config_file, cmd_options_t *options);
 void print_options(const cmd_options_t *options);
 
 #endif /* PARSER_OPTION_H */