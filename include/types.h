#ifndef TYPES_H
#define TYPES_H

// Cấu trúc lưu trữ một cặp key-value (cho phần dynamic)
typedef struct {
    char key[50];
    char value[100];
} KeyValue;

// Cấu trúc lưu trữ một thuộc tính (phần dynamic)
typedef struct {
    KeyValue *fields; // Danh sách các cặp key-value
    int field_count;
} Attribute;

// Cấu trúc lưu trữ một instruction
typedef struct {
    // Phần hardcode: Các trường cố định
    char group[50];
    char action[50];
    char action_type[20];
    char set_func[20];
    char get_func[20];
    char unset_func[20];
    char commit_func[20];
    char save_func[20];
    char node_type[20];
    char node_name[50];
    char sub_node[50];
    int max_entry;

    // Phần dynamic: Các trường thay đổi
    Attribute *attributes; // Mảng các thuộc tính
    int attr_count;
    Attribute *entry_attributes; // Mảng các entry_attributes (cho traceroute)
    int entry_attr_count;
    KeyValue *extra_fields; // Lưu các trường đặc biệt (web_id_pvc, entry_node, v.v.)
    int extra_field_count;
} Instruction;

#endif