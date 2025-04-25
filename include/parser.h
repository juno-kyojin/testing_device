#ifndef PARSER_H
#define PARSER_H

#include "types.h"

#define MAX_TEST_CASES 100

// Function to parse the test case file
int parse_test_cases(const char *filepath, TestCase *test_cases, int *test_case_count);

#endif