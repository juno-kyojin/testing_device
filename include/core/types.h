#ifndef TYPES_H
#define TYPES_H

typedef struct {
    char key[50];      // public_attr_name hoặc tên tham số
    char value[4096];  // attr_value, attr_default_value, hoặc giá trị tham số
} KeyValue;

typedef struct {
    char action[50];
    char action_type[20];
    char set_func[20];
    char get_func[20];
    char unset_func[20];
    char commit_func[20];
    char save_func[20];
    char node_type[20];
    char node_name[50];
    char node_level[50];
    char sub_node[50];
    char web_id_pvc[50];
    char web_id_entry[50];
    char entry_node[50];
    char entry_count[50];
    int max_entry;
    int max_level;
    KeyValue *attributes;
    int attr_count;
    KeyValue *entry_attributes;
    int entry_attr_count;
} Instruction;

typedef struct {
    char action[50];
    KeyValue *input_params;
    int param_count;
    KeyValue *attributes;
    int attr_count;
} TestCase;

typedef struct {
    KeyValue *fields;
    int field_count;
} Config;

typedef struct {
    Config actions;
} DeviceConfig;

// Kiểu con trỏ hàm cho xử lý hành động
typedef void (*ActionHandler)(Instruction *instr, TestCase *test_case);

// Cấu trúc cho bảng điều phối hành động
typedef struct {
    char action[50];
    ActionHandler handler;
} ActionDispatch;

#endif