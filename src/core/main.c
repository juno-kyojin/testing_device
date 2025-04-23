#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>  // Thêm để sử dụng mkdir
#include <time.h>      // Thêm để lấy thời gian
#include "file_process.h"
#include "action.h"
#include "action_registry.h"
#include "parser.h"
#include "log.h"
#include "types.h"
#include "cjson/cJSON.h"

#define MAX_TEST_CASES 100

// Ghi kết quả vào file JSON trong thư mục result với thời gian
void write_results_to_file(const char *filepath, cJSON *results) {
    // Tạo thư mục result nếu chưa tồn tại
    struct stat st = {0};
    if (stat("result", &st) == -1) {
        mkdir("result", 0755);
        log_message(LOG_LVL_DEBUG, "Created directory: result");
    }

    // Lấy thời gian hiện tại
    time_t rawtime;
    struct tm *timeinfo;
    char timestamp[20];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", timeinfo); // Định dạng: YYYYMMDDHHMMSS

    // Tách tên file từ đường dẫn
    char *filename = strrchr(filepath, '/');
    if (filename) {
        filename++; // Bỏ qua ký tự '/'
    } else {
        filename = (char *)filepath;
    }

    // Tạo tên file kết quả: result/<filename>_<timestamp>_result.json
    char result_filepath[512];
    snprintf(result_filepath, sizeof(result_filepath), "result/%s_%s_result.json", filename, timestamp);

    // Ghi kết quả vào file
    char *json_str = cJSON_Print(results);
    if (!json_str) {
        log_message(LOG_LVL_ERROR, "Failed to print results to JSON string");
        return;
    }

    FILE *fp = fopen(result_filepath, "w");
    if (!fp) {
        log_message(LOG_LVL_ERROR, "Failed to open result file %s for writing", result_filepath);
        free(json_str);
        return;
    }

    fprintf(fp, "%s", json_str);
    fclose(fp);
    free(json_str);
    log_message(LOG_LVL_DEBUG, "Results written to %s", result_filepath);
}

int main(int argc, char *argv[]) {
    init_logger();
    init_action_dispatch();

    TestCase test_cases[MAX_TEST_CASES];
    int test_case_count = 0;

    if (argc < 2) {
        DIR *dir;
        struct dirent *entry;
        dir = opendir("config");
        if (dir == NULL) {
            log_message(LOG_LVL_ERROR, "Failed to open config directory");
            cleanup_logger();
            return 1;
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, ".json") != NULL) {
                char filepath[512];
                snprintf(filepath, sizeof(filepath), "config/%s", entry->d_name);
                log_message(LOG_LVL_DEBUG, "Processing test configuration file: %s", filepath);

                int count = 0;
                if (parse_test_cases(filepath, &test_cases[test_case_count], &count) != 0) {
                    log_message(LOG_LVL_ERROR, "Failed to parse test cases from %s", filepath);
                    continue;
                }

                // Tạo mảng để lưu trữ kết quả
                cJSON *results = cJSON_CreateObject();
                cJSON *result_array = cJSON_CreateArray();
                cJSON_AddItemToObject(results, "test_cases", result_array);

                for (int i = 0; i < count; i++) {
                    execute_action(&test_cases[test_case_count + i], filepath, i, result_array);
                }

                // Ghi kết quả vào file JSON
                write_results_to_file(filepath, results);

                // Giải phóng bộ nhớ
                cJSON_Delete(results);

                test_case_count += count;
                if (test_case_count >= MAX_TEST_CASES) {
                    log_message(LOG_LVL_ERROR, "Too many test cases, stopping");
                    break;
                }
            }
        }
        closedir(dir);
    } else {
        for (int i = 1; i < argc; i++) {
            log_message(LOG_LVL_DEBUG, "Processing test configuration file: %s", argv[i]);

            int count = 0;
            if (parse_test_cases(argv[i], &test_cases[test_case_count], &count) != 0) {
                log_message(LOG_LVL_ERROR, "Failed to parse test cases from %s", argv[i]);
                continue;
            }

            // Tạo mảng để lưu trữ kết quả
            cJSON *results = cJSON_CreateObject();
            cJSON *result_array = cJSON_CreateArray();
            cJSON_AddItemToObject(results, "test_cases", result_array);

            for (int j = 0; j < count; j++) {
                execute_action(&test_cases[test_case_count + j], argv[i], j, result_array);
            }

            // Ghi kết quả vào file JSON
            write_results_to_file(argv[i], results);

            // Giải phóng bộ nhớ
            cJSON_Delete(results);

            test_case_count += count;
            if (test_case_count >= MAX_TEST_CASES) {
                log_message(LOG_LVL_ERROR, "Too many test cases, stopping");
                break;
            }
        }
    }

    cleanup_logger();
    return 0;
}