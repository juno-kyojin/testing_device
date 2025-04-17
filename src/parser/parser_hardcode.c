#include <string.h>
#include "parser_hardcode.h"

void parse_hardcode_fields(cJSON *instr_json, Instruction *instr) {
    cJSON *action = cJSON_GetObjectItem(instr_json, "action");
    cJSON *action_type = cJSON_GetObjectItem(instr_json, "action_type");
    cJSON *set_func = cJSON_GetObjectItem(instr_json, "set_func");
    cJSON *get_func = cJSON_GetObjectItem(instr_json, "get_func");
    cJSON *unset_func = cJSON_GetObjectItem(instr_json, "unset_func");
    cJSON *commit_func = cJSON_GetObjectItem(instr_json, "commit_func");
    cJSON *save_func = cJSON_GetObjectItem(instr_json, "save_func");
    cJSON *node_type = cJSON_GetObjectItem(instr_json, "node_type");
    cJSON *node_name = cJSON_GetObjectItem(instr_json, "node_name");
    cJSON *sub_node = cJSON_GetObjectItem(instr_json, "sub_node");
    cJSON *max_entry = cJSON_GetObjectItem(instr_json, "max_entry");

    if (action) strcpy(instr->action, action->valuestring);
    if (action_type) strcpy(instr->action_type, action_type->valuestring);
    if (set_func) strcpy(instr->set_func, set_func->valuestring);
    if (get_func) strcpy(instr->get_func, get_func->valuestring);
    if (unset_func) strcpy(instr->unset_func, unset_func->valuestring);
    if (commit_func) strcpy(instr->commit_func, commit_func->valuestring);
    if (save_func) strcpy(instr->save_func, save_func->valuestring);
    if (node_type) strcpy(instr->node_type, node_type->valuestring);
    if (node_name) strcpy(instr->node_name, node_name->valuestring);
    if (sub_node) strcpy(instr->sub_node, sub_node->valuestring);
    if (max_entry) instr->max_entry = max_entry->valueint;

    // Xác định group động
    if (action) {
        if (strncmp(action->valuestring, "wan", 3) == 0) {
            strcpy(instr->group, "wan");
        } else if (strncmp(action->valuestring, "ssid", 4) == 0) {
            strcpy(instr->group, "ssid");
        } else if (strncmp(action->valuestring, "portforward", 11) == 0) {
            strcpy(instr->group, "portforward");
        } else if (strncmp(action->valuestring, "speedtest", 9) == 0 ||
                   strncmp(action->valuestring, "ping", 4) == 0 ||
                   strncmp(action->valuestring, "traceroute", 10) == 0) {
            strcpy(instr->group, "diagnostic");
        } else if (strncmp(action->valuestring, "dns", 3) == 0) {
            strcpy(instr->group, "dns");
        } else if (strncmp(action->valuestring, "ddns", 4) == 0) {
            strcpy(instr->group, "ddns");
        } else if (strncmp(action->valuestring, "mesh", 4) == 0) {
            strcpy(instr->group, "mesh");
        } else if (strncmp(action->valuestring, "clientinfo", 10) == 0) {
            strcpy(instr->group, "clientinfo");
        } else if (strncmp(action->valuestring, "deviceinfo", 10) == 0) {
            strcpy(instr->group, "deviceinfo");
        } else if (strncmp(action->valuestring, "oltmodel", 8) == 0) {
            strcpy(instr->group, "oltmodel");
        } else if (strncmp(action->valuestring, "radio", 5) == 0) {
            strcpy(instr->group, "radio");
        } else if (strncmp(action->valuestring, "bndstrg", 7) == 0) {
            strcpy(instr->group, "bndstrg");
        } else if (strncmp(action->valuestring, "sip", 3) == 0) {
            strcpy(instr->group, "sip");
        } else if (strncmp(action->valuestring, "intfgroup", 9) == 0) {
            strcpy(instr->group, "intfgroup");
        } else if (strncmp(action->valuestring, "parental", 8) == 0) {
            strcpy(instr->group, "parental");
        } else if (strncmp(action->valuestring, "apstrg", 6) == 0) {
            strcpy(instr->group, "apstrg");
        } else {
            strcpy(instr->group, "unknown");
        }
    } else {
        strcpy(instr->group, "unknown");
    }
}