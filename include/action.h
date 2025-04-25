#ifndef ACTION_H
#define ACTION_H

#include "types.h"
#include "cjson/cJSON.h"

void execute_action(TestCase *test_case, const char *filepath, int index, cJSON *result_array);
void register_service(const char *service, void (*handler)(TestCase *, const char *, int, cJSON *));

#endif