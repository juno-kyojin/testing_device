#ifndef PARSER_H
#define PARSER_H

#include "types.h"

// Đọc và parse JSON từ file
int parser_data(const char *file_path, Instruction **instructions);

#endif