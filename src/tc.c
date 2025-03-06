/**
 * @file tc.c
 * @brief Implementation of test case execution functionality
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include <net/if.h>
 #include <netinet/in.h>
 #include <netinet/ip_icmp.h>
 
 #include "../include/tc.h"
 #include "../include/log.h"
 #include "../include/file_process.h"
 
 // Default timeout values in milliseconds
 #define TCP_CONNECT_TIMEOUT 5000
 #define UDP_TIMEOUT 3000
 #define ICMP_TIMEOUT 2000
 #define HTTP_TIMEOUT 10000
 
 // Test execution statistics
 static uint64_t g_total_tests = 0;
 static uint64_t g_successful_tests = 0;
 static uint64_t g_failed_tests = 0;
 static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
 
 // Results file handle
 static FILE* results_file = NULL;
 static pthread_mutex_t results_mutex = PTHREAD_MUTEX_INITIALIZER;
 
 // Forward declarations for internal functions
 static int tcp_test(const char* target, uint16_t port, int timeout_ms);
 static int udp_test(const char* target, uint16_t port, int timeout_ms);
 static int icmp_test(const char* target, int timeout_ms);
 static int http_test(const char* target, uint16_t port, int timeout_ms, bool use_ssl);
 static int write_test_result(const test_case_t* test_case, int result);
 
 int init_test_execution_engine(void) {
     LOG_INFO(LOG_CAT_TEST_ENGINE, "Initializing test execution engine");
     
     pthread_mutex_lock(&stats_mutex);
     g_total_tests = 0;
     g_successful_tests = 0;
     g_failed_tests = 0;
     pthread_mutex_unlock(&stats_mutex);
     
     // Create results file with timestamp
     char results_filename[256];
     time_t now = time(NULL);
     struct tm* timeinfo = localtime(&now);
     char timestamp[32];
     strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
     snprintf(results_filename, sizeof(results_filename), "results_mass_batch_%s.txt", timestamp);
     
     pthread_mutex_lock(&results_mutex);
     results_file = open_file(results_filename, FILE_MODE_WRITE);
     pthread_mutex_unlock(&results_mutex);
     
     if (!results_file) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to create results file: %s", results_filename);
         return -1;
     }
     
     LOG_INFO(LOG_CAT_TEST_ENGINE, "Test execution engine initialized, results file: %s", 
              results_filename);
     return 0;
 }
 
 int cleanup_test_execution_engine(void) {
     LOG_INFO(LOG_CAT_TEST_ENGINE, "Cleaning up test execution engine");
     
     pthread_mutex_lock(&results_mutex);
     if (results_file) {
         fclose(results_file);
         results_file = NULL;
     }
     pthread_mutex_unlock(&results_mutex);
     
     pthread_mutex_lock(&stats_mutex);
     LOG_INFO(LOG_CAT_TEST_ENGINE, "Test execution summary: Total=%lu, Success=%lu, Failed=%lu",
             (unsigned long)g_total_tests, 
             (unsigned long)g_successful_tests, 
             (unsigned long)g_failed_tests);
     pthread_mutex_unlock(&stats_mutex);
     
     return 0;
 }
 
 int execute_test_case(const test_case_t* test_case) {
     if (!test_case) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Invalid test case parameter");
         return TC_RESULT_INVALID_PARAM;
     }
     
     LOG_DEBUG(LOG_CAT_TEST_ENGINE, "Executing test case ID %lu, target: %s, protocol: %d",
              (unsigned long)test_case->id, test_case->target, test_case->protocol);
     
     int result = TC_RESULT_UNKNOWN_ERROR;
     
     // Execute based on protocol
     switch (test_case->protocol) {
         case PROTOCOL_TCP:
             result = tcp_test(test_case->target, test_case->port, TCP_CONNECT_TIMEOUT);
             break;
             
         case PROTOCOL_UDP:
             result = udp_test(test_case->target, test_case->port, UDP_TIMEOUT);
             break;
             
         case PROTOCOL_ICMP:
             result = icmp_test(test_case->target, ICMP_TIMEOUT);
             break;
             
         case PROTOCOL_HTTP:
             result = http_test(test_case->target, test_case->port, HTTP_TIMEOUT, false);
             break;
             
         case PROTOCOL_HTTPS:
             result = http_test(test_case->target, test_case->port, HTTP_TIMEOUT, true);
             break;
             
         default:
             LOG_ERROR(LOG_CAT_TEST_ENGINE, "Unsupported protocol for test case ID %lu",
                     (unsigned long)test_case->id);
             result = TC_RESULT_INVALID_PARAM;
     }
     
     // Update statistics
     pthread_mutex_lock(&stats_mutex);
     g_total_tests++;
     if (result == TC_RESULT_SUCCESS) {
         g_successful_tests++;
     } else {
         g_failed_tests++;
     }
     pthread_mutex_unlock(&stats_mutex);
     
     // Write result to file
     write_test_result(test_case, result);
     
     LOG_DEBUG(LOG_CAT_TEST_ENGINE, "Test case ID %lu completed with result: %d",
              (unsigned long)test_case->id, result);
     
     return result;
 }
 
 int execute_test_case_by_network(const test_case_t* test_case) {
     if (!test_case) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Invalid test case parameter");
         return TC_RESULT_INVALID_PARAM;
     }
     
     // Execute based on network type
     switch (test_case->network_type) {
         case NETWORK_TYPE_LAN:
             return execute_lan_test(test_case);
             
         case NETWORK_TYPE_WAN:
             return execute_wan_test(test_case);
             
         default:
             LOG_ERROR(LOG_CAT_TEST_ENGINE, "Unsupported network type for test case ID %lu",
                     (unsigned long)test_case->id);
             return TC_RESULT_INVALID_PARAM;
     }
 }
 
 int execute_lan_test(const test_case_t* test_case) {
     if (!test_case) {
         return TC_RESULT_INVALID_PARAM;
     }
     
     LOG_DEBUG(LOG_CAT_TEST_ENGINE, "Executing LAN test for case ID %lu",
              (unsigned long)test_case->id);
     
     // For LAN tests, we may need to use specific network interface
     // For now, we'll just execute the standard test
     return execute_test_case(test_case);
 }
 
 int execute_wan_test(const test_case_t* test_case) {
     if (!test_case) {
         return TC_RESULT_INVALID_PARAM;
     }
     
     LOG_DEBUG(LOG_CAT_TEST_ENGINE, "Executing WAN test for case ID %lu",
              (unsigned long)test_case->id);
     
     // For WAN tests, we may need to route through WAN interface
     // For now, we'll just execute the standard test
     return execute_test_case(test_case);
 }
 
 int report_execution_progress(uint64_t processed, uint64_t total, const char* batch_id) {
     if (!batch_id) {
         return -1;
     }
     
     // Calculate progress percentage
     double percentage = (total > 0) ? (100.0 * processed / total) : 0.0;
     
     // Log progress
     LOG_INFO(LOG_CAT_TEST_ENGINE, "[PROGRESS] %.2f%% - Processed %lu/%lu tests - Batch: %s",
              percentage, (unsigned long)processed, (unsigned long)total, batch_id);
     
     // Could also send progress to PC App here using get_data module
     // For now, we just log it
     
     return 0;
 }
 
 int log_execution_progress(uint64_t processed, uint64_t total, const char* batch_id) {
     // This is similar to report_execution_progress but only logs to the log file
     return report_execution_progress(processed, total, batch_id);
 }
 
 static int tcp_test(const char* target, uint16_t port, int timeout_ms) {
     if (!target || port == 0) {
         return TC_RESULT_INVALID_PARAM;
     }
     
     struct sockaddr_in server_addr;
     int sock = socket(AF_INET, SOCK_STREAM, 0);
     if (sock < 0) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to create socket for TCP test: %s",
                 strerror(errno));
         return TC_RESULT_NETWORK_ERROR;
     }
     
     // Set non-blocking mode for timeout support
     int flags = fcntl(sock, F_GETFL, 0);
     fcntl(sock, F_SETFL, flags | O_NONBLOCK);
     
     // Prepare server address
     memset(&server_addr, 0, sizeof(server_addr));
     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(port);
     
     // Convert hostname to IP if needed
     struct hostent* host = gethostbyname(target);
     if (host) {
         memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
     } else {
         // Try direct IP address
         if (inet_pton(AF_INET, target, &server_addr.sin_addr) <= 0) {
             LOG_ERROR(LOG_CAT_TEST_ENGINE, "Invalid address or hostname: %s", target);
             close(sock);
             return TC_RESULT_NETWORK_ERROR;
         }
     }
     
     // Attempt to connect
     int ret = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
     
     if (ret < 0 && errno != EINPROGRESS) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to connect to %s:%d - %s", 
                  target, port, strerror(errno));
         close(sock);
         return TC_RESULT_NETWORK_ERROR;
     }
     
     // Wait for connection with timeout
     fd_set write_fds;
     struct timeval tv;
     
     FD_ZERO(&write_fds);
     FD_SET(sock, &write_fds);
     tv.tv_sec = timeout_ms / 1000;
     tv.tv_usec = (timeout_ms % 1000) * 1000;
     
     ret = select(sock + 1, NULL, &write_fds, NULL, &tv);
     
     if (ret <= 0) {
         // Timeout or error
         LOG_WARN(LOG_CAT_TEST_ENGINE, "TCP connection to %s:%d timed out after %d ms",
                 target, port, timeout_ms);
         close(sock);
         return TC_RESULT_TIMEOUT;
     }
     
     // Check if connection was successful
     int error = 0;
     socklen_t len = sizeof(error);
     if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to connect to %s:%d - %s", 
                  target, port, strerror(error));
         close(sock);
         return TC_RESULT_NETWORK_ERROR;
     }
     
     // Connection successful
     close(sock);
     return TC_RESULT_SUCCESS;
 }
 
 static int udp_test(const char* target, uint16_t port, int timeout_ms) {
     if (!target || port == 0) {
         return TC_RESULT_INVALID_PARAM;
     }
     
     struct sockaddr_in server_addr;
     int sock = socket(AF_INET, SOCK_DGRAM, 0);
     if (sock < 0) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to create socket for UDP test: %s",
                 strerror(errno));
         return TC_RESULT_NETWORK_ERROR;
     }
     
     // Set timeout
     struct timeval tv;
     tv.tv_sec = timeout_ms / 1000;
     tv.tv_usec = (timeout_ms % 1000) * 1000;
     setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
     
     // Prepare server address
     memset(&server_addr, 0, sizeof(server_addr));
     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(port);
     
     // Convert hostname to IP if needed
     struct hostent* host = gethostbyname(target);
     if (host) {
         memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
     } else {
         // Try direct IP address
         if (inet_pton(AF_INET, target, &server_addr.sin_addr) <= 0) {
             LOG_ERROR(LOG_CAT_TEST_ENGINE, "Invalid address or hostname: %s", target);
             close(sock);
             return TC_RESULT_NETWORK_ERROR;
         }
     }
     
     // Send a test packet (simple "ping" message)
     const char* message = "PING";
     if (sendto(sock, message, strlen(message), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to send UDP packet to %s:%d - %s", 
                  target, port, strerror(errno));
         close(sock);
         return TC_RESULT_NETWORK_ERROR;
     }
     
     // UDP is connectionless, so we can't really verify connection
     // For a more thorough test, we'd need a server that responds
     // For now, we'll just consider the send successful
     
     close(sock);
     return TC_RESULT_SUCCESS;
 }
 
 static int icmp_test(const char* target, int timeout_ms) {
     // ICMP requires raw sockets which need root privileges
     // This is a simplified version for illustration
     
     LOG_WARN(LOG_CAT_TEST_ENGINE, "ICMP test requires root privileges, using system ping instead");
     
     // Use system ping command as a fallback
     char cmd[256];
     snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s > /dev/null 2>&1",
              timeout_ms / 1000, target);
     
     int ret = system(cmd);
     if (ret == 0) {
         return TC_RESULT_SUCCESS;
     } else {
         LOG_WARN(LOG_CAT_TEST_ENGINE, "Ping to %s failed with return code %d", target, ret);
         return TC_RESULT_NETWORK_ERROR;
     }
 }
 
 static int http_test(const char* target, uint16_t port, int timeout_ms, bool use_ssl) {
     // For a full HTTP test, we'd need HTTP/HTTPS libraries
     // This is a simplified version using a TCP connection test
     
     if (use_ssl) {
         // Default HTTPS port if not specified
         if (port == 0) port = 443;
         
         LOG_INFO(LOG_CAT_TEST_ENGINE, "HTTPS test using TCP connection to %s:%d", target, port);
     } else {
         // Default HTTP port if not specified
         if (port == 0) port = 80;
         
         LOG_INFO(LOG_CAT_TEST_ENGINE, "HTTP test using TCP connection to %s:%d", target, port);
     }
     
     // For now, we just test the TCP connection
     return tcp_test(target, port, timeout_ms);
 }
 
 static int write_test_result(const test_case_t* test_case, int result) {
     if (!test_case) {
         return -1;
     }
     
     pthread_mutex_lock(&results_mutex);
     
     if (!results_file) {
         pthread_mutex_unlock(&results_mutex);
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Results file not open");
         return -1;
     }
     
     // Get current time
     time_t now = time(NULL);
     struct tm* timeinfo = localtime(&now);
     char timestamp[32];
     strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
     
     // Write result to file
     fprintf(results_file, "[%s] Test ID: %lu, Target: %s, Protocol: %d, Port: %d, Network: %s, Result: %s\n",
            timestamp,
            (unsigned long)test_case->id,
            test_case->target,
            test_case->protocol,
            test_case->port,
            (test_case->network_type == NETWORK_TYPE_LAN) ? "LAN" : "WAN",
            (result == TC_RESULT_SUCCESS) ? "SUCCESS" : "FAILURE");
     
     // Flush to ensure data is written to disk
     fflush(results_file);
     
     pthread_mutex_unlock(&results_mutex);
     return 0;
 }