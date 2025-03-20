/**
 * @file test_tc.c
 * @brief Test program for tc.c functionality
 * @version 0.1
 * @date 2025-03-20
 */

#include "../include/tc.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// Test cases
test_case_t test_cases[3];
test_result_info_t results[3];

// Setup test cases
void setup_test_cases() {
    // Test case 1: Successful ping to localhost
    memset(&test_cases[0], 0, sizeof(test_case_t));
    strcpy(test_cases[0].id, "PING_01");
    strcpy(test_cases[0].name, "Ping to localhost");
    strcpy(test_cases[0].description, "Test ping to localhost");
    strcpy(test_cases[0].target, "127.0.0.1");
    test_cases[0].type = TEST_PING;
    test_cases[0].network_type = NETWORK_LAN;
    test_cases[0].timeout = 5000;
    test_cases[0].enabled = true;
    test_cases[0].params.ping.count = 3;
    test_cases[0].params.ping.size = 56;
    test_cases[0].params.ping.interval = 100;
    test_cases[0].params.ping.ipv6 = false;
    
    // Test case 2: Ping to unreachable host
    memset(&test_cases[1], 0, sizeof(test_case_t));
    strcpy(test_cases[1].id, "PING_02");
    strcpy(test_cases[1].name, "Ping to unreachable host");
    strcpy(test_cases[1].description, "Test ping to unreachable host");
    strcpy(test_cases[1].target, "8.8.8.8");  // An unlikely local address
    test_cases[1].type = TEST_PING;
    test_cases[1].network_type = NETWORK_LAN;
    test_cases[1].timeout = 3000;
    test_cases[1].enabled = true;
    test_cases[1].params.ping.count = 2;
    test_cases[1].params.ping.size = 56;
    test_cases[1].params.ping.interval = 100;
    test_cases[1].params.ping.ipv6 = false;
    
    // Test case 3: Non-ping test (should be skipped)
    memset(&test_cases[2], 0, sizeof(test_case_t));
    strcpy(test_cases[2].id, "THROUGHPUT_01");
    strcpy(test_cases[2].name, "Throughput test");
    strcpy(test_cases[2].description, "Test throughput to localhost");
    strcpy(test_cases[2].target, "127.0.0.1");
    test_cases[2].type = TEST_THROUGHPUT;
    test_cases[2].network_type = NETWORK_LAN;
    test_cases[2].timeout = 5000;
    test_cases[2].enabled = true;
}

// Test execute_ping_test function
void test_execute_ping_test() {
    printf("\n===== Test execute_ping_test =====\n");
    
    // Test ping to localhost
    memset(&results[0], 0, sizeof(test_result_info_t));
    
    // Chạy lệnh ping trực tiếp để so sánh
    printf("Running direct ping command for comparison:\n");
    system("ping -c 3 -s 56 -i 0.1 127.0.0.1");
    printf("\n");
    
    int ret = execute_ping_test(&test_cases[0], &results[0]);
    
    printf("Test ping to localhost: %s\n", ret == 0 ? "PASSED" : "FAILED");
    printf("  Status: %s\n", test_result_status_to_string(results[0].status));
    printf("  Details: %s\n", results[0].result_details);
    printf("  Execution time: %.2f ms\n", results[0].execution_time);
    
    if (results[0].status == TEST_RESULT_SUCCESS) {
        printf("  Packets sent: %d\n", results[0].data.ping.packets_sent);
        printf("  Packets received: %d\n", results[0].data.ping.packets_received);
        printf("  Packet loss: %.1f%%\n", results[0].data.ping.packet_loss);
        printf("  RTT min/avg/max: %.3f/%.3f/%.3f ms\n", 
               results[0].data.ping.min_rtt, 
               results[0].data.ping.avg_rtt, 
               results[0].data.ping.max_rtt);
    }
    
    // Test ping to unreachable host
    memset(&results[1], 0, sizeof(test_result_info_t));
    ret = execute_ping_test(&test_cases[1], &results[1]);
    
    printf("\nTest ping to unreachable host: %s\n", ret == 0 ? "PASSED" : "FAILED");
    printf("  Status: %s\n", test_result_status_to_string(results[1].status));
    printf("  Details: %s\n", results[1].result_details);
    printf("  Execution time: %.2f ms\n", results[1].execution_time);
}

// Test execute_test_case function
void test_execute_test_case() {
    printf("\n===== Test execute_test_case =====\n");
    
    // Test Ping
    memset(&results[0], 0, sizeof(test_result_info_t));
    int ret = execute_test_case(&test_cases[0], &results[0]);
    
    printf("Test ping case: %s\n", ret == 0 ? "PASSED" : "FAILED");
    printf("  Status: %s\n", test_result_status_to_string(results[0].status));
    printf("  Details: %s\n", results[0].result_details);
    
    // Test non-ping (should be skipped/error)
    memset(&results[2], 0, sizeof(test_result_info_t));
    ret = execute_test_case(&test_cases[2], &results[2]);
    
    printf("\nTest non-ping case: %s\n", ret == 0 ? "PASSED" : "FAILED");
    printf("  Status: %s\n", test_result_status_to_string(results[2].status));
    printf("  Details: %s\n", results[2].result_details);
}

// Test execute_test_case_by_network function
void test_execute_test_case_by_network() {
    printf("\n===== Test execute_test_case_by_network =====\n");
    
    // Test on LAN network (should work)
    memset(&results[0], 0, sizeof(test_result_info_t));
    int ret = execute_test_case_by_network(&test_cases[0], NETWORK_LAN, &results[0]);
    
    printf("Test ping on LAN: %s\n", ret == 0 ? "PASSED" : "FAILED");
    printf("  Status: %s\n", test_result_status_to_string(results[0].status));
    
    // Test on WAN network (should fail due to network type mismatch)
    memset(&results[0], 0, sizeof(test_result_info_t));
    ret = execute_test_case_by_network(&test_cases[0], NETWORK_WAN, &results[0]);
    
    printf("\nTest ping on WAN (should fail): %s\n", ret == -1 ? "PASSED" : "FAILED");
    printf("  Status: %s\n", test_result_status_to_string(results[0].status));
    printf("  Details: %s\n", results[0].result_details);
}

// Test generate_summary_report function
void test_generate_summary_report() {
    printf("\n===== Test generate_summary_report =====\n");
    
    // Run all tests first to populate results
    execute_test_case(&test_cases[0], &results[0]);
    execute_test_case(&test_cases[1], &results[1]);
    execute_test_case(&test_cases[2], &results[2]);
    
    // Generate report
    char report_file[64] = "test_report.json";
    int ret = generate_summary_report(results, 3, report_file);
    
    printf("Generate report: %s\n", ret == 0 ? "PASSED" : "FAILED");
    printf("Report saved to: %s\n", report_file);
    
    // Verify file exists
    if (access(report_file, F_OK) == 0) {
        printf("Report file exists: YES\n");
        
        // Read back file to verify content (simple check)
        FILE *f = fopen(report_file, "r");
        if (f) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), f) != NULL) {
                printf("Report content starts with: %s", buffer);
            }
            fclose(f);
        }
    } else {
        printf("Report file exists: NO\n");
    }
}

int main() {
    // Initialize logger
    set_log_level(LOG_LVL_DEBUG);
    set_log_file("test_tc.log");
    
    printf("Running tc.c tests...\n");
    
    // Setup test cases
    setup_test_cases();
    
    // Run tests
    test_execute_ping_test();
    test_execute_test_case();
    test_execute_test_case_by_network();
    test_generate_summary_report();
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
