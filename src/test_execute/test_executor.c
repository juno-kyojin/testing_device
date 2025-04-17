#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test_executor.h"
#include "parser.h"
#include "log.h"
#include "wan.h"

typedef void (*ActionHandler)(Instruction *instr);

typedef struct {
    const char *group;
    const char *action;
    ActionHandler handler;
} ActionMap;

// Danh sách động để lưu action handler
#define MAX_ACTIONS 100
static ActionMap action_map[MAX_ACTIONS];
static int action_count = 0;

void register_action_handler(const char *group, const char *action, ActionHandler handler) {
    if (action_count >= MAX_ACTIONS) {
        log_message(LOG_LVL_ERROR, "Cannot register more actions, limit reached");
        return;
    }

    action_map[action_count].group = group;
    action_map[action_count].action = action;
    action_map[action_count].handler = handler;
    action_count++;
}

static void init_action_map(void) {
    // Chỉ đăng ký các action của nhóm wan
    register_action_handler("wan", "wanViewConfig", wan_view_config);
    register_action_handler("wan", "wanViewStatus", wan_view_status);
    register_action_handler("wan", "wanPPPoECreate", wan_pppoe_create);
    register_action_handler("wan", "wanIPoECreate", wan_ipoe_create);
    register_action_handler("wan", "wanBridgeCreate", wan_bridge_create);
    register_action_handler("wan", "wanPPPoEEdit", wan_pppoe_edit);
    register_action_handler("wan", "wanIPoEEdit", wan_ipoe_edit);
    register_action_handler("wan", "wanBridgeEdit", wan_bridge_edit);
    register_action_handler("wan", "wanRemove", wan_remove);
}

static void process_instructions(Instruction *instructions, int count) {
    for (int i = 0; i < count; i++) {
        Instruction *instr = &instructions[i];
        for (int j = 0; j < action_count; j++) {
            if (strcmp(instr->group, action_map[j].group) == 0 && 
                strcmp(instr->action, action_map[j].action) == 0) {
                log_message(LOG_LVL_DEBUG, "Sẽ gọi hàm kiểm thử: %s (group: %s)", instr->action, instr->group);
                action_map[j].handler(instr);
                break;
            }
        }
    }
}

int execute_tests(const char *test_file_path) {
    init_action_map();

    Instruction *instructions;
    int instr_count = parser_data(test_file_path, &instructions);
    if (instr_count == 0) {
        log_message(LOG_LVL_ERROR, "Failed to parse test file: %s", test_file_path);
        return -1;
    }

    process_instructions(instructions, instr_count);

    for (int i = 0; i < instr_count; i++) {
        free(instructions[i].extra_fields);
        for (int j = 0; j < instructions[i].attr_count; j++) {
            free(instructions[i].attributes[j].fields);
        }
        free(instructions[i].attributes);
    }
    free(instructions);

    return 0;
}