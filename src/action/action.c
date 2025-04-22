#include <stdio.h>
#include <string.h>
#include "../include/action/action.h"
#include "../include/action/action_registry.h"
#include "../include/core/log.h"
// Bảng điều phối hành động
static ActionDispatch action_table[100]; // Giới hạn 100 hành động, có thể tăng nếu cần
static int action_count = 0;

// Hàm chung để xử lý các action không có handler riêng
static void default_action_handler(Instruction *instr, TestCase *test_case) {
    if (strcmp(instr->action_type, "set") == 0 || strcmp(instr->action_type, "set_ssid") == 0 || 
        strcmp(instr->action_type, "set_radio") == 0 || strcmp(instr->action_type, "set_voip") == 0 ||
        strcmp(instr->action_type, "set_interface") == 0) {
        log_message(LOG_LVL_DEBUG, "Preparing to call %s for node %s_%s", 
                    instr->set_func, instr->node_name, instr->sub_node);
        for (int i = 0; i < test_case->attr_count; i++) {
            if (strlen(test_case->attributes[i].value) > 0) {
                log_message(LOG_LVL_DEBUG, "Set attribute %s = %s", 
                            test_case->attributes[i].key, test_case->attributes[i].value);
            } else {
                log_message(LOG_LVL_WARN, "Attribute %s has no value", test_case->attributes[i].key);
            }
        }
        // Log input_params nếu có
        if (test_case->param_count > 0) {
            log_message(LOG_LVL_DEBUG, "Input parameters:");
            for (int i = 0; i < test_case->param_count; i++) {
                log_message(LOG_LVL_DEBUG, "Param %s = %s", 
                            test_case->input_params[i].key, test_case->input_params[i].value);
            }
        }
        // TODO: Thêm logic gọi tcapi_set khi có API
    } else if (strcmp(instr->action_type, "get") == 0 || strcmp(instr->action_type, "getall") == 0 || 
               strcmp(instr->action_type, "get_radio") == 0 || strcmp(instr->action_type, "get_voip") == 0 ||
               strcmp(instr->action_type, "getall_voip") == 0) {
        log_message(LOG_LVL_DEBUG, "Preparing to call %s for node %s_%s", 
                    instr->get_func, instr->node_name, instr->sub_node);
        for (int i = 0; i < test_case->attr_count; i++) {
            log_message(LOG_LVL_DEBUG, "Get attribute %s", test_case->attributes[i].key);
        }
        // Log input_params nếu có
        if (test_case->param_count > 0) {
            log_message(LOG_LVL_DEBUG, "Input parameters:");
            for (int i = 0; i < test_case->param_count; i++) {
                log_message(LOG_LVL_DEBUG, "Param %s = %s", 
                            test_case->input_params[i].key, test_case->input_params[i].value);
            }
        }
        // TODO: Thêm logic gọi tcapi_get khi có API
    } else if (strcmp(instr->action_type, "unset") == 0 || strcmp(instr->action_type, "unset_interface") == 0) {
        log_message(LOG_LVL_DEBUG, "Preparing to call %s for node %s_%s", 
                    instr->unset_func, instr->node_name, instr->sub_node);
        for (int i = 0; i < test_case->attr_count; i++) {
            log_message(LOG_LVL_DEBUG, "Unset attribute %s", test_case->attributes[i].key);
        }
        // Log input_params nếu có
        if (test_case->param_count > 0) {
            log_message(LOG_LVL_DEBUG, "Input parameters:");
            for (int i = 0; i < test_case->param_count; i++) {
                log_message(LOG_LVL_DEBUG, "Param %s = %s", 
                            test_case->input_params[i].key, test_case->input_params[i].value);
            }
        }
        // TODO: Thêm logic gọi tcapi_unset khi có API
    } else if (strcmp(instr->action_type, "diagnostic") == 0) {
        log_message(LOG_LVL_DEBUG, "Preparing diagnostic action with %s and %s", 
                    instr->set_func, instr->get_func);
        for (int i = 0; i < test_case->attr_count; i++) {
            if (strlen(test_case->attributes[i].value) > 0) {
                log_message(LOG_LVL_DEBUG, "Diagnostic attribute %s = %s", 
                            test_case->attributes[i].key, test_case->attributes[i].value);
            } else {
                log_message(LOG_LVL_DEBUG, "Diagnostic attribute %s", test_case->attributes[i].key);
            }
        }
        // Log input_params nếu có
        if (test_case->param_count > 0) {
            log_message(LOG_LVL_DEBUG, "Input parameters:");
            for (int i = 0; i < test_case->param_count; i++) {
                log_message(LOG_LVL_DEBUG, "Param %s = %s", 
                            test_case->input_params[i].key, test_case->input_params[i].value);
            }
        }
        // TODO: Thêm logic gọi API diagnostic khi có
    } else {
        log_message(LOG_LVL_ERROR, "Unknown action_type: %s", instr->action_type);
    }
}

// Hàm đăng ký hành động
void register_action(const char *action, ActionHandler handler) {
    if (action_count >= 100) {
        log_message(LOG_LVL_ERROR, "Action dispatch table is full");
        return;
    }
    strncpy(action_table[action_count].action, action, sizeof(action_table[action_count].action) - 1);
    action_table[action_count].action[sizeof(action_table[action_count].action) - 1] = '\0';
    action_table[action_count].handler = handler;
    action_count++;
}

// Hàm tra cứu handler cho hành động
static ActionHandler get_action_handler(const char *action) {
    for (int i = 0; i < action_count; i++) {
        if (strcmp(action_table[i].action, action) == 0) {
            return action_table[i].handler;
        }
    }
    return NULL;
}

void execute_action(Instruction *instr, TestCase *test_case) {
    log_message(LOG_LVL_DEBUG, "Executing action: %s (type: %s)", instr->action, instr->action_type);

    // Tra cứu handler từ bảng điều phối
    ActionHandler handler = get_action_handler(instr->action);
    if (handler) {
        handler(instr, test_case);
    } else {
        // Nếu không có handler riêng, dùng xử lý chung
        default_action_handler(instr, test_case);
    }
}