#ifndef ACTION_H
#define ACTION_H

#include "../core/types.h"

void execute_action(Instruction *instr, TestCase *test_case);
void register_action(const char *action, ActionHandler handler);

#endif