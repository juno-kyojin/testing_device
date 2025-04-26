/**
 * @file ping.h
 * @brief Header file for the ping test case handler
 *
 * This header file defines the interface for the ping test case handler, which
 * executes ping test cases by sending ICMP packets to a specified host and recording
 * the results. It includes the definition of the `PingResult` structure and declares
 * functions for parsing, executing, and logging ping test cases.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see ping.c
 * @see action.h
 */
#ifndef PING_H
#define PING_H

#include "types.h"
#include "cjson/cJSON.h"
#include <time.h>

/**
 * @struct PingResult
 * @brief Structure to store the results of a ping test case
 *
 * This structure holds the results of a ping test case, including the number of packets
 * transmitted and received, the packet loss percentage, the average latency, and the
 * status of the test case ("pass" or "fail").
 *
 * @var PingResult::packets_received
 *      Number of ICMP packets received from the host.
 * @var PingResult::packets_transmitted
 *      Number of ICMP packets transmitted to the host.
 * @var PingResult::avg_time
 *      Average round-trip time (latency) in milliseconds.
 * @var PingResult::packet_loss
 *      Percentage of packets lost during the test (0.0 to 100.0).
 * @var PingResult::status
 *      Status of the test case ("pass" or "fail").
 */
typedef struct {
    int packets_received;
    int packets_transmitted;
    float avg_time;
    float packet_loss;
    char status[16];
} PingResult;

/**
 * @brief Parse the host parameter from a test case in a JSON file
 *
 * This function reads a JSON file containing test cases, extracts the host parameter
 * for the test case at the specified index, and updates the result JSON object.
 * If the host parameter is missing or the JSON file is invalid, an error is logged,
 * and the result JSON object retains its default values (indicating failure).
 *
 * @param filepath Path to the JSON file containing the test cases.
 * @param index Zero-based index of the test case within the JSON file's test case array.
 * @param host Buffer to store the extracted host string (e.g., "google.com").
 * @param host_size Maximum size of the host buffer.
 * @param result_json Pointer to the `cJSON` object where the test case result is stored.
 * @return int
 *         - 0 if the host was successfully parsed.
 *         - -1 if an error occurred (e.g., file not found, invalid JSON, missing host).
 */
int parse_host(const char *filepath, int index, char *host, size_t host_size, cJSON *result_json);

/**
 * @brief Execute a ping command and collect the results
 *
 * This function executes the `ping` command on the specified host, sending 5 ICMP
 * packets with a timeout of 3 seconds. It collects the results, including the number
 * of packets transmitted, packets received, packet loss percentage, and average latency.
 * The status of the test case is determined based on the packet loss:
 * - "pass" if all packets are received.
 * - "fail" otherwise.
 *
 * @param host The host to ping (e.g., "google.com"). Must be a null-terminated string.
 * @param result Pointer to a `PingResult` structure where the results will be stored.
 * @return int
 *         - 0 if the ping command executed successfully.
 *         - -1 if an error occurred (e.g., failed to execute the command).
 */
int ping(const char *host, PingResult *result);

/**
 * @brief Log the results of a ping test case
 *
 * This function logs the results of a ping test case, including the pass percentage,
 * the average latency, and the status of the test case. It also logs the total execution
 * time of the test case.
 *
 * @param result Pointer to the `PingResult` structure containing the test results.
 * @param start_time Start time of the test case execution.
 * @param end_time End time of the test case execution.
 * @return None
 */
void log_result(const PingResult *result, time_t start_time, time_t end_time);

/**
 * @brief Execute a ping test case and store the results
 *
 * This function handles the execution of a ping test case. It parses the host parameter
 * from the test case, executes the ping command, logs the results, and stores the results
 * in the provided JSON array. If any step fails (e.g., missing host, failed command),
 * the result is marked as failed with an appropriate error message.
 *
 * @param test_case Pointer to the `TestCase` structure containing the service and action information.
 * @param filepath Path to the JSON file containing the test case definitions.
 * @param index Zero-based index of the test case within the JSON file's test case array.
 * @param result_array Pointer to a `cJSON` array where the test case results will be stored.
 * @return None
 */
void execute_ping(TestCase *test_case, const char *filepath, int index, cJSON *result_array);

#endif