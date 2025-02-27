#ifndef GET_DATA_H
#define GET_DATA_H

#include <stdbool.h>
#include <stdio.h>

// Tạo báo cáo tổng hợp từ `logs/system.log`
void generate_test_report(const char *log_file, const char *report_file);

// Gửi kết quả kiểm thử về PC qua SCP
void send_test_results_to_pc(const char *pc_ip, const char *pc_user, const char *destination_path);
#endif // GET_DATA_H
