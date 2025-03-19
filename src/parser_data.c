#include "parser_data.h"
#include "file_process.h"
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>

// Hàm helper để kiểm tra và xử lý dữ liệu cJSON
static cJSON* get_json_object_safe(cJSON *obj, const char *field) {
    cJSON *item = cJSON_GetObjectItem(obj, field);
    return item;
}

static int get_json_int_safe(cJSON *obj, const char *field, int default_value) {
    cJSON *item = get_json_object_safe(obj, field);
    return (item && cJSON_IsNumber(item)) ? item->valueint : default_value;
}

static const char* get_json_string_safe(cJSON *obj, const char *field, const char *default_value) {
    cJSON *item = get_json_object_safe(obj, field);
    return (item && cJSON_IsString(item)) ? item->valuestring : default_value;
}

bool read_json_test_cases(const char *json_file, test_case_t **test_cases, int *count) {
    char *json_str = NULL;
    size_t size;
    // Đọc nội dung file JSON vào chuỗi
    if (read_file(json_file, &json_str, &size) != 0) {
        return false;
    }

    // Phân tích chuỗi JSON bằng cJSON
    cJSON *root = cJSON_Parse(json_str);
    free(json_str); // Giải phóng chuỗi sau khi phân tích
    if (!root) {
        return false;
    }

    // Lấy số lượng phần tử trong mảng JSON
    int array_size = cJSON_GetArraySize(root);
    *test_cases = (test_case_t *)malloc(array_size * sizeof(test_case_t));
    if (!*test_cases) {
        cJSON_Delete(root);
        return false;
    }

    // Chuyển đổi từng phần tử JSON thành test_case_t
    *count = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, root) {
        test_case_t *tc = &(*test_cases)[*count];
        
        // Chuyển đổi item thành chuỗi JSON để phù hợp với API
        char *item_str = cJSON_Print(item);
        if (item_str) {
            if (json_to_test_case(item_str, tc)) {
                (*count)++;
            }
            free(item_str);
        }
    }

    cJSON_Delete(root); // Giải phóng đối tượng cJSON
    return true;
}

bool json_to_test_case(const char *json_str, test_case_t *test_case) {
    if (!json_str || !test_case) {
        return false;
    }
    
    // Parse chuỗi JSON
    cJSON *item = cJSON_Parse(json_str);
    if (!item) {
        return false;
    }
    
    bool result = false;
    
    // Lấy các trường cơ bản từ JSON
    cJSON *id = get_json_object_safe(item, "id");
    cJSON *type = get_json_object_safe(item, "type");
    cJSON *network_type = get_json_object_safe(item, "network_type");
    cJSON *name = get_json_object_safe(item, "name");
    cJSON *description = get_json_object_safe(item, "description");
    cJSON *target = get_json_object_safe(item, "target");
    cJSON *timeout = get_json_object_safe(item, "timeout");
    cJSON *enabled = get_json_object_safe(item, "enabled");
    cJSON *params = get_json_object_safe(item, "params");

    // Kiểm tra các trường bắt buộc
    if (!id || !cJSON_IsString(id) || 
        !type || !cJSON_IsNumber(type) || 
        !network_type || !cJSON_IsNumber(network_type) || 
        !name || !cJSON_IsString(name) || 
        !description || !cJSON_IsString(description) || 
        !target || !cJSON_IsString(target) || 
        !timeout || !cJSON_IsNumber(timeout) || 
        !enabled || !params) {
        cJSON_Delete(item);
        return false;
    }

    // Kiểm tra phạm vi enum
    int type_value = type->valueint;
    int network_type_value = network_type->valueint;
    
    if (type_value < TEST_PING || type_value > TEST_OTHER || 
        network_type_value < NETWORK_LAN || network_type_value > NETWORK_BOTH) {
        cJSON_Delete(item);
        return false;
    }

    // Gán giá trị vào cấu trúc test_case_t
    memset(test_case, 0, sizeof(test_case_t)); // Xóa bộ nhớ trước khi gán giá trị
    
    strncpy(test_case->id, id->valuestring, sizeof(test_case->id) - 1);
    test_case->id[sizeof(test_case->id) - 1] = '\0';
    
    test_case->type = (test_type_t)type_value;
    test_case->network_type = (network_type_t)network_type_value;
    
    strncpy(test_case->name, name->valuestring, sizeof(test_case->name) - 1);
    test_case->name[sizeof(test_case->name) - 1] = '\0';
    
    strncpy(test_case->description, description->valuestring, sizeof(test_case->description) - 1);
    test_case->description[sizeof(test_case->description) - 1] = '\0';
    
    strncpy(test_case->target, target->valuestring, sizeof(test_case->target) - 1);
    test_case->target[sizeof(test_case->target) - 1] = '\0';
    
    test_case->timeout = timeout->valueint;
    test_case->enabled = cJSON_IsTrue(enabled);

    // Xử lý tham số params theo loại test
    switch (test_case->type) {
        case TEST_PING: {
            test_case->params.ping.count = get_json_int_safe(params, "count", 0);
            test_case->params.ping.size = get_json_int_safe(params, "size", 0);
            break;
        }
        case TEST_THROUGHPUT: {
            test_case->params.throughput.duration = get_json_int_safe(params, "duration", 0);
            const char *protocol = get_json_string_safe(params, "protocol", "");
            strncpy(test_case->params.throughput.protocol, protocol, sizeof(test_case->params.throughput.protocol) - 1);
            test_case->params.throughput.protocol[sizeof(test_case->params.throughput.protocol) - 1] = '\0';
            break;
        }
        case TEST_VLAN: {
            test_case->params.vlan.vlan_id = get_json_int_safe(params, "vlan_id", 0);
            break;
        }
        case TEST_SECURITY: {
            const char *method = get_json_string_safe(params, "method", "");
            strncpy(test_case->params.security.method, method, sizeof(test_case->params.security.method) - 1);
            test_case->params.security.method[sizeof(test_case->params.security.method) - 1] = '\0';
            break;
        }
        default:
            break; // Có thể mở rộng cho các loại test khác
    }

    // Kiểm tra nếu có dữ liệu bổ sung
    cJSON *extra = get_json_object_safe(item, "extra_data");
    if (extra) {
        char *extra_str = cJSON_Print(extra);
        if (extra_str) {
            test_case->extra_data_size = strlen(extra_str) + 1;
            test_case->extra_data = malloc(test_case->extra_data_size);
            if (test_case->extra_data) {
                memcpy(test_case->extra_data, extra_str, test_case->extra_data_size);
            }
            free(extra_str);
        }
    } else {
        test_case->extra_data = NULL;
        test_case->extra_data_size = 0;
    }
    
    result = true;
    cJSON_Delete(item);
    return result;
}

void free_test_cases(test_case_t *test_cases, int count) {
    if (test_cases) {
        for (int i = 0; i < count; i++) {
            if (test_cases[i].extra_data) {
                free(test_cases[i].extra_data);
                test_cases[i].extra_data = NULL;
            }
        }
        free(test_cases);
    }
}

bool filter_test_cases_by_network(const test_case_t *test_cases, int count, network_type_t network_type, test_case_t **filtered_test_cases, int *filtered_count) {
    if (!test_cases || count <= 0 || !filtered_test_cases || !filtered_count) {
        return false;
    }

    // Đếm số lượng test case phù hợp
    int filtered_size = 0;
    for (int i = 0; i < count; i++) {
        if (test_cases[i].network_type == network_type || test_cases[i].network_type == NETWORK_BOTH) {
            filtered_size++;
        }
    }

    // Cấp phát bộ nhớ cho mảng lọc
    *filtered_test_cases = (test_case_t *)malloc(filtered_size * sizeof(test_case_t));
    if (!*filtered_test_cases) {
        return false;
    }

    // Sao chép các test case phù hợp
    *filtered_count = 0;
    for (int i = 0; i < count; i++) {
        if (test_cases[i].network_type == network_type || test_cases[i].network_type == NETWORK_BOTH) {
            test_case_t *src = (test_case_t *)&test_cases[i];
            test_case_t *dest = &(*filtered_test_cases)[*filtered_count];
            
            // Sao chép các trường cơ bản
            *dest = *src;
            
            // Deep copy cho extra_data nếu cần
            if (src->extra_data != NULL && src->extra_data_size > 0) {
                dest->extra_data = malloc(src->extra_data_size);
                if (dest->extra_data) {
                    memcpy(dest->extra_data, src->extra_data, src->extra_data_size);
                } else {
                    // Lỗi cấp phát - thiết lập về NULL để tránh lỗi
                    dest->extra_data = NULL;
                    dest->extra_data_size = 0;
                }
            } else {
                dest->extra_data = NULL;
                dest->extra_data_size = 0;
            }
            
            (*filtered_count)++;
        }
    }

    return true;
}

bool test_case_to_json(const test_case_t *test_case, char *json_buffer, size_t buffer_size) {
    if (!test_case || !json_buffer || buffer_size <= 0) {
        return false;
    }
    
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return false;
    }
    
    // Thêm các trường cơ bản
    cJSON_AddStringToObject(root, "id", test_case->id);
    cJSON_AddNumberToObject(root, "type", test_case->type);
    cJSON_AddNumberToObject(root, "network_type", test_case->network_type);
    cJSON_AddStringToObject(root, "name", test_case->name);
    cJSON_AddStringToObject(root, "description", test_case->description);
    cJSON_AddStringToObject(root, "target", test_case->target);
    cJSON_AddNumberToObject(root, "timeout", test_case->timeout);
    cJSON_AddBoolToObject(root, "enabled", test_case->enabled);
    
    // Tạo đối tượng params
    cJSON *params = cJSON_CreateObject();
    if (!params) {
        cJSON_Delete(root);
        return false;
    }
    
    // Thêm các trường params tùy theo loại test
    switch (test_case->type) {
        case TEST_PING:
            cJSON_AddNumberToObject(params, "count", test_case->params.ping.count);
            cJSON_AddNumberToObject(params, "size", test_case->params.ping.size);
            break;
        case TEST_THROUGHPUT:
            cJSON_AddNumberToObject(params, "duration", test_case->params.throughput.duration);
            cJSON_AddStringToObject(params, "protocol", test_case->params.throughput.protocol);
            break;
        case TEST_VLAN:
            cJSON_AddNumberToObject(params, "vlan_id", test_case->params.vlan.vlan_id);
            break;
        case TEST_SECURITY:
            cJSON_AddStringToObject(params, "method", test_case->params.security.method);
            break;
        default:
            break;
    }
    
    cJSON_AddItemToObject(root, "params", params);
    
    // Thêm extra_data nếu có
    if (test_case->extra_data && test_case->extra_data_size > 0) {
        cJSON *extra_data = cJSON_Parse((char*)test_case->extra_data);
        if (extra_data) {
            cJSON_AddItemToObject(root, "extra_data", extra_data);
        }
    }
    
    // Chuyển đối tượng JSON thành chuỗi
    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    if (!json_str) {
        return false;
    }
    
    // Sao chép chuỗi JSON vào buffer
    if (strlen(json_str) < buffer_size) {
        strcpy(json_buffer, json_str);
        free(json_str);
        return true;
    } else {
        strncpy(json_buffer, json_str, buffer_size - 1);
        json_buffer[buffer_size - 1] = '\0';
        free(json_str);
        return false; // Buffer không đủ lớn
    }
}