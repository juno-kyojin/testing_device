#ifndef PARSER_HARDCODE_H
#define PARSER_HARDCODE_H

#include "cjson/cJSON.h"
#include "types.h"

void parse_hardcode_fields(cJSON *instr_json, Instruction *instr);

#endif