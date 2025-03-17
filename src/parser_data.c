#include "parser_data.h"
#include "file_process.h"
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>

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
        if (json_to_test_case(item, tc)) {
            (*count)++;
        }
    }

    cJSON_Delete(root); // Giải phóng đối tượng cJSON
    return true;
}

bool json_to_test_case(cJSON *item, test_case_t *test_case) {
    // Lấy các trường cơ bản từ JSON
    cJSON *id = cJSON_GetObjectItem(item, "id");
    cJSON *type = cJSON_GetObjectItem(item, "type");
    cJSON *network_type = cJSON_GetObjectItem(item, "network_type");
    cJSON *name = cJSON_GetObjectItem(item, "name");
    cJSON *description = cJSON_GetObjectItem(item, "description");
    cJSON *target = cJSON_GetObjectItem(item, "target");
    cJSON *timeout = cJSON_GetObjectItem(item, "timeout");
    cJSON *enabled = cJSON_GetObjectItem(item, "enabled");
    cJSON *params = cJSON_GetObjectItem(item, "params");

    // Kiểm tra các trường bắt buộc
    if (!id || !type || !network_type || !name || !description || !target || !timeout || !enabled || !params) {
        return false;
    }

    // Gán giá trị vào cấu trúc test_case_t
    strncpy(test_case->id, id->valuestring, sizeof(test_case->id) - 1);
    test_case->type = (test_type_t)type->valueint;
    test_case->network_type = (network_type_t)network_type->valueint;
    strncpy(test_case->name, name->valuestring, sizeof(test_case->name) - 1);
    strncpy(test_case->description, description->valuestring, sizeof(test_case->description) - 1);
    strncpy(test_case->target, target->valuestring, sizeof(test_case->target) - 1);
    test_case->timeout = timeout->valueint;
    test_case->enabled = enabled->valueint != 0;

    // Xử lý tham số params theo loại test
    switch (test_case->type) {
        case TEST_PING:
            test_case->params.ping.count = cJSON_GetObjectItem(params, "count")->valueint;
            test_case->params.ping.size = cJSON_GetObjectItem(params, "size")->valueint;
            break;
        case TEST_THROUGHPUT:
            test_case->params.throughput.duration = cJSON_GetObjectItem(params, "duration")->valueint;
            strncpy(test_case->params.throughput.protocol, cJSON_GetObjectItem(params, "protocol")->valuestring, sizeof(test_case->params.throughput.protocol) - 1);
            break;
        case TEST_VLAN:
            test_case->params.vlan.vlan_id = cJSON_GetObjectItem(params, "vlan_id")->valueint;
            break;
        case TEST_SECURITY:
            strncpy(test_case->params.security.method, cJSON_GetObjectItem(params, "method")->valuestring, sizeof(test_case->params.security.method) - 1);
            break;
        default:
            break; // Có thể mở rộng cho các loại test khác
    }

    test_case->extra_data = NULL; // Chưa xử lý extra_data
    test_case->extra_data_size = 0;
    return true;
}

void free_test_cases(test_case_t *test_cases, int count) {
    if (test_cases) {
        for (int i = 0; i < count; i++) {
            if (test_cases[i].extra_data) {
                free(test_cases[i].extra_data);
            }
        }
        free(test_cases);
    }
}

bool filter_test_cases_by_network(const test_case_t *test_cases, int count, network_type_t network_type, test_case_t **filtered_test_cases, int *filtered_count) {
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
            memcpy(&(*filtered_test_cases)[*filtered_count], &test_cases[i], sizeof(test_case_t));
            (*filtered_count)++;
        }
    }

    return true;
}