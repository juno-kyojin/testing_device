#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser/parser_hardcode.h"
#include "cjson/cJSON.h"
#include "core/log.h"

void parse_hardcode_fields(cJSON *instr_json, Instruction *instr) {
    cJSON *action = cJSON_GetObjectItem(instr_json, "action");
    if (action && cJSON_IsString(action)) {
        strncpy(instr->action, action->valuestring, sizeof(instr->action) - 1);
        instr->action[sizeof(instr->action) - 1] = '\0';
    }

    cJSON *action_type = cJSON_GetObjectItem(instr_json, "action_type");
    if (action_type && cJSON_IsString(action_type)) {
        strncpy(instr->action_type, action_type->valuestring, sizeof(instr->action_type) - 1);
        instr->action_type[sizeof(instr->action_type) - 1] = '\0';
    }

    cJSON *set_func = cJSON_GetObjectItem(instr_json, "set_func");
    if (set_func && cJSON_IsString(set_func)) {
        strncpy(instr->set_func, set_func->valuestring, sizeof(instr->set_func) - 1);
        instr->set_func[sizeof(instr->set_func) - 1] = '\0';
    }

    cJSON *get_func = cJSON_GetObjectItem(instr_json, "get_func");
    if (get_func && cJSON_IsString(get_func)) {
        strncpy(instr->get_func, get_func->valuestring, sizeof(instr->get_func) - 1);
        instr->get_func[sizeof(instr->get_func) - 1] = '\0';
    }

    cJSON *unset_func = cJSON_GetObjectItem(instr_json, "unset_func");
    if (unset_func && cJSON_IsString(unset_func)) {
        strncpy(instr->unset_func, unset_func->valuestring, sizeof(instr->unset_func) - 1);
        instr->unset_func[sizeof(instr->unset_func) - 1] = '\0';
    }

    cJSON *commit_func = cJSON_GetObjectItem(instr_json, "commit_func");
    if (commit_func && cJSON_IsString(commit_func)) {
        strncpy(instr->commit_func, commit_func->valuestring, sizeof(instr->commit_func) - 1);
        instr->commit_func[sizeof(instr->commit_func) - 1] = '\0';
    }

    cJSON *save_func = cJSON_GetObjectItem(instr_json, "save_func");
    if (save_func && cJSON_IsString(save_func)) {
        strncpy(instr->save_func, save_func->valuestring, sizeof(instr->save_func) - 1);
        instr->save_func[sizeof(instr->save_func) - 1] = '\0';
    }

    cJSON *node_type = cJSON_GetObjectItem(instr_json, "node_type");
    if (node_type && cJSON_IsString(node_type)) {
        strncpy(instr->node_type, node_type->valuestring, sizeof(instr->node_type) - 1);
        instr->node_type[sizeof(instr->node_type) - 1] = '\0';
    }

    cJSON *node_name = cJSON_GetObjectItem(instr_json, "node_name");
    if (node_name && cJSON_IsString(node_name)) {
        strncpy(instr->node_name, node_name->valuestring, sizeof(instr->node_name) - 1);
        instr->node_name[sizeof(instr->node_name) - 1] = '\0';
    }

    cJSON *node_level = cJSON_GetObjectItem(instr_json, "node_level");
    if (node_level && cJSON_IsString(node_level)) {
        strncpy(instr->node_level, node_level->valuestring, sizeof(instr->node_level) - 1);
        instr->node_level[sizeof(instr->node_level) - 1] = '\0';
    }

    cJSON *sub_node = cJSON_GetObjectItem(instr_json, "sub_node");
    if (sub_node && cJSON_IsString(sub_node)) {
        strncpy(instr->sub_node, sub_node->valuestring, sizeof(instr->sub_node) - 1);
        instr->sub_node[sizeof(instr->sub_node) - 1] = '\0';
    }

    cJSON *web_id_pvc = cJSON_GetObjectItem(instr_json, "web_id_pvc");
    if (web_id_pvc && cJSON_IsString(web_id_pvc)) {
        strncpy(instr->web_id_pvc, web_id_pvc->valuestring, sizeof(instr->web_id_pvc) - 1);
        instr->web_id_pvc[sizeof(instr->web_id_pvc) - 1] = '\0';
    }

    cJSON *web_id_entry = cJSON_GetObjectItem(instr_json, "web_id_entry");
    if (web_id_entry && cJSON_IsString(web_id_entry)) {
        strncpy(instr->web_id_entry, web_id_entry->valuestring, sizeof(instr->web_id_entry) - 1);
        instr->web_id_entry[sizeof(instr->web_id_entry) - 1] = '\0';
    }

    cJSON *entry_node = cJSON_GetObjectItem(instr_json, "entry_node");
    if (entry_node && cJSON_IsString(entry_node)) {
        strncpy(instr->entry_node, entry_node->valuestring, sizeof(instr->entry_node) - 1);
        instr->entry_node[sizeof(instr->entry_node) - 1] = '\0';
    }

    cJSON *entry_count = cJSON_GetObjectItem(instr_json, "entry_count");
    if (entry_count && cJSON_IsString(entry_count)) {
        strncpy(instr->entry_count, entry_count->valuestring, sizeof(instr->entry_count) - 1);
        instr->entry_count[sizeof(instr->entry_count) - 1] = '\0';
    }

    cJSON *max_entry = cJSON_GetObjectItem(instr_json, "max_entry");
    if (max_entry && cJSON_IsNumber(max_entry)) {
        instr->max_entry = max_entry->valueint;
    }

    cJSON *max_level = cJSON_GetObjectItem(instr_json, "max_level");
    if (max_level && cJSON_IsNumber(max_level)) {
        instr->max_level = max_level->valueint;
    }
}