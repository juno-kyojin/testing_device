#ifndef TC_API_H
#define TC_API_H

#include "parser_data.h"
#include "tc.h"
#include <cjson/cJSON.h>

/**
 * @brief Thực thi test case
 * 
 * @param test_case Con trỏ đến test case
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_test_case(test_case_t *test_case, test_result_info_t *result);

/**
 * @brief Thực thi ping test
 * 
 * @param test_case Con trỏ đến test case
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_ping_test(test_case_t *test_case, test_result_info_t *result);

/**
 * @brief Thực thi speedtest test
 * 
 * @param test_case Con trỏ đến test case
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_speedtest_test(test_case_t *test_case, test_result_info_t *result);

/**
 * @brief Thực thi test case dựa trên loại mạng
 * 
 * @param test_case Con trỏ đến test case
 * @param network_type Loại mạng để thực thi (LAN hoặc WAN)
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_test_case_by_network(test_case_t *test_case, network_type_t network_type, test_result_info_t *result);

/**
 * @brief Thực thi instruction dựa trên input parameters
 * 
 * @param instruction Con trỏ đến instruction
 * @param input_params Con trỏ đến input parameters dạng JSON
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_instruction(const instruction_t *instruction, cJSON *input_params);

/**
 * @brief Lấy giá trị thuộc tính từ node
 * 
 * @param node Tên node
 * @param entry Tên entry
 * @param attribute Tên thuộc tính
 * @param value Buffer lưu giá trị
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int tcapi_get(const char *node, const char *entry, const char *attribute, char *value);

/**
 * @brief Đặt giá trị thuộc tính cho node
 * 
 * @param node Tên node
 * @param entry Tên entry
 * @param attribute Tên thuộc tính
 * @param value Giá trị cần đặt
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int tcapi_set(const char *node, const char *entry, const char *attribute, const char *value);

/**
 * @brief Lưu cấu hình
 * 
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int tcapi_save(void);

/**
 * @brief Commit cấu hình chẩn đoán
 * 
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int ai_diagnostic_commit(void);

#endif /* TC_API_H */