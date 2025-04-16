#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "tc.h"
#include "parser_data.h"
#include <cjson/cJSON.h>

/**
 * @brief Cấu trúc chứa thông tin về plugin test
 */
typedef struct {
    char name[64];               // Tên plugin
    char description[256];       // Mô tả plugin
    char version[32];            // Phiên bản
    char action_name[32];        // Tên action (để mapping với instruction)
    char original_action[32];    // Tên action gốc (trước khi đổi tên để tránh trùng lặp)
    test_type_t test_type;       // Loại test mà plugin hỗ trợ
    void *handle;                // Handle của plugin động
    
    // Con trỏ đến hàm thực thi test
    int (*execute_test)(test_case_t *test_case, test_result_info_t *result, cJSON *input_params);
    
    // Con trỏ đến hàm khởi tạo (nếu cần)
    int (*initialize)(void);
    
    // Con trỏ đến hàm giải phóng tài nguyên (nếu cần)
    void (*cleanup)(void);
} plugin_info_t;

/**
 * @brief Khởi tạo hệ thống plugin
 * 
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int plugin_manager_init(void);

/**
 * @brief Nạp tất cả plugin từ thư mục plugin
 * 
 * @param plugin_dir Đường dẫn đến thư mục chứa plugin
 * @return int Số lượng plugin đã nạp thành công
 */
int load_plugins(const char *plugin_dir);

/**
 * @brief Tìm plugin dựa trên tên action
 * 
 * @param action_name Tên action cần tìm
 * @return plugin_info_t* Con trỏ đến plugin nếu tìm thấy, NULL nếu không
 */
plugin_info_t* find_plugin_by_action(const char *action_name);

/**
 * @brief Thực thi test thông qua plugin
 * 
 * @param action_name Tên action của plugin
 * @param test_case Con trỏ đến test case
 * @param result Con trỏ đến biến lưu kết quả
 * @param input_params Tham số đầu vào bổ sung (nếu có)
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int plugin_execute_test(const char *action_name, test_case_t *test_case, 
                         test_result_info_t *result, cJSON *input_params);

/**
 * @brief Giải phóng tài nguyên của hệ thống plugin
 */
void plugin_manager_cleanup(void);

/**
 * @brief Tạo cấu trúc plugin_info rỗng
 * 
 * @return plugin_info_t Cấu trúc plugin_info rỗng
 */
plugin_info_t create_empty_plugin(void);

/**
 * @brief Lấy danh sách các plugin đã nạp
 * 
 * @param count Con trỏ đến biến lưu số lượng plugin
 * @return plugin_info_t* Mảng chứa thông tin các plugin
 */
plugin_info_t* get_loaded_plugins(int *count);

#endif /* PLUGIN_MANAGER_H */
