/**
 * @file get_data.c
 * @brief Implementation of report management and data transfer
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <errno.h>
 
 #include "../include/get_data.h"
 #include "../include/file_process.h"
 #include "../include/log.h"
 
 // SSH/SCP commands
 #define SCP_CMD_TEMPLATE "scp -o StrictHostKeyChecking=no -P %d %s %s %s@%s:%s"
 #define SSH_CMD_TEMPLATE "ssh -o StrictHostKeyChecking=no -p %d %s %s@%s \"%s\""
 
 // Static SSH configuration
 static ssh_config_t ssh_cfg = {0};
 static int ssh_initialized = 0;
 
 int init_ssh_config(const char* host, int port, const char* username, 
                     const char* password, const char* key_path, 
                     const char* remote_dir) {
     if (!host || !username || !remote_dir) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Invalid SSH configuration parameters");
         return -1;
     }
     
     strncpy(ssh_cfg.host, host, sizeof(ssh_cfg.host) - 1);
     ssh_cfg.port = port;
     strncpy(ssh_cfg.username, username, sizeof(ssh_cfg.username) - 1);
     
     if (password) {
         strncpy(ssh_cfg.password, password, sizeof(ssh_cfg.password) - 1);
     } else {
         ssh_cfg.password[0] = '\0';
     }
     
     if (key_path) {
         strncpy(ssh_cfg.key_path, key_path, sizeof(ssh_cfg.key_path) - 1);
     } else {
         ssh_cfg.key_path[0] = '\0';
     }
     
     strncpy(ssh_cfg.remote_dir, remote_dir, sizeof(ssh_cfg.remote_dir) - 1);
     
     ssh_initialized = 1;
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "SSH configuration initialized for %s@%s:%d", 
              username, host, port);
     
     return 0;
 }
 
 int generate_test_report_with_progress(const char* log_file, const char* report_file,
                                       uint64_t total_cases, const char* batch_id,
                                       const char* network_type) {
     if (!log_file || !report_file || !batch_id || !network_type) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Invalid parameters for report generation");
         return -1;
     }
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "Generating test report from %s to %s", 
              log_file, report_file);
     
     // Open log file for reading
     FILE* log_fp = open_file(log_file, FILE_MODE_READ);
     if (!log_fp) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Failed to open log file: %s", log_file);
         return -1;
     }
     
     // Open report file for writing
     FILE* report_fp = open_file(report_file, FILE_MODE_WRITE);
     if (!report_fp) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Failed to open report file: %s", report_file);
         fclose(log_fp);
         return -1;
     }
     
     // Get current time for the report header
     time_t now = time(NULL);
     struct tm* timeinfo = localtime(&now);
     char timestamp[64];
     strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
     
     // Write report header
     fprintf(report_fp, "=== Test Execution Report ===\n");
     fprintf(report_fp, "Date: %s\n", timestamp);
     fprintf(report_fp, "Batch ID: %s\n", batch_id);
     fprintf(report_fp, "Network Type: %s\n", network_type);
     fprintf(report_fp, "Total Test Cases: %lu\n\n", (unsigned long)total_cases);
     
     // Parse log file to gather statistics
     char line[1024];
     uint64_t success_count = 0;
     uint64_t failure_count = 0;
     uint64_t error_count = 0;
     uint64_t processed_count = 0;
     
     while (fgets(line, sizeof(line), log_fp) != NULL) {
         // Count successes
         if (strstr(line, "[SUCCESS]") || strstr(line, "test passed")) {
             success_count++;
         }
         // Count failures
         else if (strstr(line, "[FAIL]") || strstr(line, "test failed")) {
             failure_count++;
         }
         // Count errors
         else if (strstr(line, "[ERROR]")) {
             error_count++;
         }
         
         // Parse progress information
         if (strstr(line, "[PROGRESS]")) {
             char* percent_str = strstr(line, "Processed");
             if (percent_str) {
                 if (sscanf(percent_str, "Processed %lu/%lu", 
                            &processed_count, (unsigned long*)&total_cases) != 2) {
                     // If parsing fails, keep the previous values
                 }
             }
         }
     }
     
     // Write summary statistics to report
     fprintf(report_fp, "=== Summary ===\n");
     fprintf(report_fp, "Processed: %lu/%lu (%.2f%%)\n", 
             (unsigned long)processed_count, (unsigned long)total_cases,
             (total_cases > 0) ? (100.0 * processed_count / total_cases) : 0.0);
     fprintf(report_fp, "Successful: %lu (%.2f%%)\n", 
             (unsigned long)success_count, 
             (processed_count > 0) ? (100.0 * success_count / processed_count) : 0.0);
     fprintf(report_fp, "Failed: %lu (%.2f%%)\n", 
             (unsigned long)failure_count, 
             (processed_count > 0) ? (100.0 * failure_count / processed_count) : 0.0);
     fprintf(report_fp, "Errors: %lu\n\n", (unsigned long)error_count);
     
     // Add timestamp for report creation
     fprintf(report_fp, "Report generated at: %s\n", timestamp);
     
     // Close files
     fclose(log_fp);
     fclose(report_fp);
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "Test report generation completed: %s", report_file);
     
     return 0;
 }
 
 int send_compressed_results_to_pc(const char* results_file, const char* report_file,
                                  bool compress) {
     if (!ssh_initialized) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "SSH not initialized. Call init_ssh_config first.");
         return -1;
     }
     
     if (!results_file || !report_file) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Invalid file paths for sending");
         return -1;
     }
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "Preparing to send results and report to PC App");
     
     char compressed_results[256];
     char compressed_report[256];
     int ret = 0;
     
     // Compress files if needed
     if (compress) {
         snprintf(compressed_results, sizeof(compressed_results), "%s.gz", results_file);
         snprintf(compressed_report, sizeof(compressed_report), "%s.gz", report_file);
         
         LOG_INFO(LOG_CAT_RESULT_MANAGER, "Compressing results file: %s -> %s", 
                  results_file, compressed_results);
         if (compress_file(results_file, compressed_results, COMPRESS_LEVEL_BEST) != 0) {
             LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Failed to compress results file");
             return -1;
         }
         
         LOG_INFO(LOG_CAT_RESULT_MANAGER, "Compressing report file: %s -> %s", 
                  report_file, compressed_report);
         if (compress_file(report_file, compressed_report, COMPRESS_LEVEL_BEST) != 0) {
             LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Failed to compress report file");
             return -1;
         }
     } else {
         strncpy(compressed_results, results_file, sizeof(compressed_results) - 1);
         strncpy(compressed_report, report_file, sizeof(compressed_report) - 1);
     }
     
     // Prepare SCP command for results file
     char key_option[256] = "";
     if (strlen(ssh_cfg.key_path) > 0) {
         snprintf(key_option, sizeof(key_option), "-i %s", ssh_cfg.key_path);
     }
     
     char scp_cmd1[1024];
     snprintf(scp_cmd1, sizeof(scp_cmd1), SCP_CMD_TEMPLATE,
              ssh_cfg.port, key_option, compress ? compressed_results : results_file,
              ssh_cfg.username, ssh_cfg.host, ssh_cfg.remote_dir);
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "Sending results file via SCP: %s", 
              compress ? compressed_results : results_file);
     ret = system(scp_cmd1);
     if (ret != 0) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Failed to send results file via SCP, return code: %d", ret);
         return -1;
     }
     
     // Prepare SCP command for report file
     char scp_cmd2[1024];
     snprintf(scp_cmd2, sizeof(scp_cmd2), SCP_CMD_TEMPLATE,
              ssh_cfg.port, key_option, compress ? compressed_report : report_file,
              ssh_cfg.username, ssh_cfg.host, ssh_cfg.remote_dir);
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "Sending report file via SCP: %s", 
              compress ? compressed_report : report_file);
     ret = system(scp_cmd2);
     if (ret != 0) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Failed to send report file via SCP, return code: %d", ret);
         return -1;
     }
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "Successfully sent results and report to PC App");
     
     // Clean up temporary compressed files if created
     if (compress) {
         if (unlink(compressed_results) != 0) {
             LOG_WARN(LOG_CAT_RESULT_MANAGER, "Could not remove temporary compressed results file");
         }
         if (unlink(compressed_report) != 0) {
             LOG_WARN(LOG_CAT_RESULT_MANAGER, "Could not remove temporary compressed report file");
         }
     }
     
     return 0;
 }
 
 int send_progress_to_pc(uint64_t processed, uint64_t total, const char* batch_id) {
     if (!ssh_initialized) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "SSH not initialized. Call init_ssh_config first.");
         return -1;
     }
     
     if (total == 0 || !batch_id) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Invalid parameters for sending progress");
         return -1;
     }
     
     // Calculate progress percentage
     double percentage = 100.0 * processed / total;
     
     // Prepare progress message
     char progress_msg[256];
     snprintf(progress_msg, sizeof(progress_msg), 
              "echo 'PROGRESS_UPDATE: %.2f%% - Processed %lu/%lu - Batch: %s'",
              percentage, (unsigned long)processed, (unsigned long)total, batch_id);
     
     // Prepare SSH command
     char key_option[256] = "";
     if (strlen(ssh_cfg.key_path) > 0) {
         snprintf(key_option, sizeof(key_option), "-i %s", ssh_cfg.key_path);
     }
     
     char ssh_cmd[1024];
     snprintf(ssh_cmd, sizeof(ssh_cmd), SSH_CMD_TEMPLATE,
              ssh_cfg.port, key_option, ssh_cfg.username, ssh_cfg.host, progress_msg);
     
     // Execute SSH command
     LOG_DEBUG(LOG_CAT_RESULT_MANAGER, "Sending progress update: %.2f%%", percentage);
     int ret = system(ssh_cmd);
     if (ret != 0) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "Failed to send progress via SSH, return code: %d", ret);
         return -1;
     }
     
     return 0;
 }
 
 int test_ssh_connection(void) {
     if (!ssh_initialized) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "SSH not initialized. Call init_ssh_config first.");
         return -1;
     }
     
     // Prepare SSH test command
     char key_option[256] = "";
     if (strlen(ssh_cfg.key_path) > 0) {
         snprintf(key_option, sizeof(key_option), "-i %s", ssh_cfg.key_path);
     }
     
     char ssh_cmd[1024];
     snprintf(ssh_cmd, sizeof(ssh_cmd), SSH_CMD_TEMPLATE,
              ssh_cfg.port, key_option, ssh_cfg.username, ssh_cfg.host, "echo 'Connection test successful'");
     
     // Execute SSH command
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "Testing SSH connection to %s@%s", ssh_cfg.username, ssh_cfg.host);
     int ret = system(ssh_cmd);
     if (ret != 0) {
         LOG_ERROR(LOG_CAT_RESULT_MANAGER, "SSH connection test failed, return code: %d", ret);
         return -1;
     }
     
     LOG_INFO(LOG_CAT_RESULT_MANAGER, "SSH connection test successful");
     return 0;
 }