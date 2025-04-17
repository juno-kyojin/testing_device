#ifndef TEST_EXECUTOR_H
#define TEST_EXECUTOR_H

#include "types.h"

// Đăng ký action handler
typedef void (*ActionHandler)(Instruction *instr);
void register_action_handler(const char *group, const char *action, ActionHandler handler);

// Thực thi các test case từ file JSON
int execute_tests(const char *test_file_path);

#endif