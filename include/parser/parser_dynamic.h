#ifndef PARSER_DYNAMIC_H
#define PARSER_DYNAMIC_H

#include "cjson/cJSON.h"
#include "core/types.h"

void parse_dynamic_fields(cJSON *instr_json, Instruction *instr);

#endif