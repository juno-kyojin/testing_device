#include <stdio.h>
#include <string.h>
#include "action.h"
#include "action_registry.h"
#include "log.h"

// Bảng điều phối hành động
static ActionDispatch action_table[100]; // Giới hạn 100 dịch vụ
static int action_count = 0;

// Hàm đăng ký dịch vụ
void register_service(const char *service, ActionHandler handler) {
    if (action_count >= 100) {
        log_message(LOG_LVL_ERROR, "Action dispatch table is full");
        return;
    }
    strncpy(action_table[action_count].service, service, sizeof(action_table[action_count].service) - 1);
    action_table[action_count].service[sizeof(action_table[action_count].service) - 1] = '\0';
    action_table[action_count].handler = handler;
    action_count++;
}

// Hàm tra cứu handler cho dịch vụ
static ActionHandler get_action_handler(const char *service) {
    for (int i = 0; i < action_count; i++) {
        if (strcmp(action_table[i].service, service) == 0) {
            return action_table[i].handler;
        }
    }
    return NULL;
}

void execute_action(TestCase *test_case, const char *filepath, int index, cJSON *result_array) {
    log_message(LOG_LVL_DEBUG, "Executing action: %s (service: %s, action: %s)", 
                test_case->action[0] ? test_case->action : test_case->service, 
                test_case->service, 
                test_case->action[0] ? test_case->action : "default");

    ActionHandler handler = get_action_handler(test_case->service);
    if (handler) {
        handler(test_case, filepath, index, result_array);
    } else {
        log_message(LOG_LVL_ERROR, "No handler found for service: %s", test_case->service);
    }
}