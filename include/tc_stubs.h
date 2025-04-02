#ifndef TC_STUBS_H
#define TC_STUBS_H

#include "parser_data.h"
#include "tc.h"
#include "cjson/cJSON.h"

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

#endif /* TC_STUBS_H */
