#ifndef PARSER_DYNAMIC_H
#define PARSER_DYNAMIC_H

#include "cjson/cJSON.h"
#include "types.h"

// Parse một object JSON thành danh sách key-value
void parse_json_object(cJSON *obj, KeyValue **fields, int *field_count);

// Parse mảng (attributes hoặc entry_attributes)
void parse_dynamic_array(cJSON *array_json, Attribute **attrs, int *attr_count);

// Parse các trường dynamic (web_id_pvc, entry_node, v.v.)
void parse_dynamic_fields(cJSON *instr_json, Instruction *instr);

#endif