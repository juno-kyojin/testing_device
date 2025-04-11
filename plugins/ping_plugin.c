#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "plugin_api.h"
#include "log.h"  // Add this include for LOG_LVL_* constants

// Plugin information
static const plugin_info_t PLUGIN_INFO = {
    .api_version = PLUGIN_API_VERSION,
    .name = "ping_test",
    .version = "1.0.0",
    .description = "ICMP ping test plugin",
    .author = "Testing Device Team"
};

// Plugin capabilities
static const plugin_capabilities_t PLUGIN_CAPABILITIES = {
    .action_name = "ping",
    .node_name = "Ping",
    .has_input_params = true,
    .has_custom_output = false
};

// Plugin context
static plugin_context_t *plugin_ctx = NULL;

// Helper function to parse ping output - renamed to avoid conflict with tc.h
static int plugin_parse_ping_result(const char *output, ping_data_t *data) {
    if (!output || !data) return -1;
    
    // Initialize data
    memset(data, 0, sizeof(ping_data_t));
    
    // Debug: Log the complete ping output for troubleshooting
    plugin_ctx->log_message(LOG_LVL_DEBUG, "Ping raw output: %s", output);
    
    // Extract packets sent and received using direct string scanning
    const char *stats_line = strstr(output, "statistics");
    if (stats_line) {
        // Find the line with packets transmitted
        const char *tx_line = strstr(stats_line, "packets transmitted");
        if (tx_line) {
            // Scan backwards to find the number before "packets transmitted"
            const char *p = tx_line;
            while (p > output && *(p-1) != '\n') p--;
            
            // Now p points to the beginning of the line with "packets transmitted"
            int sent = 0, received = 0;
            float loss = 0.0f;
            
            // Use sscanf with the full line pattern
            if (sscanf(p, "%d packets transmitted, %d received", &sent, &received) == 2) {
                data->packets_sent = sent;
                data->packets_received = received;
                
                // Calculate packet loss percentage
                if (sent > 0) {
                    data->packet_loss = 100.0f * (sent - received) / sent;
                }
                
                plugin_ctx->log_message(LOG_LVL_DEBUG, "Successfully parsed packets: sent=%d, received=%d, loss=%.1f%%", 
                                       data->packets_sent, data->packets_received, data->packet_loss);
            } else {
                plugin_ctx->log_message(LOG_LVL_WARN, "Failed to match packets sent/received pattern with standard format");
                
                // Try alternative format with explicit packet loss percentage
                int loss_int = 0;
                if (sscanf(p, "%d packets transmitted, %d received, %d%% packet loss", 
                          &sent, &received, &loss_int) >= 2) {
                    data->packets_sent = sent;
                    data->packets_received = received;
                    data->packet_loss = (float)loss_int;
                    plugin_ctx->log_message(LOG_LVL_DEBUG, "Parsed using alternative format: sent=%d, received=%d, loss=%.1f%%", 
                                          data->packets_sent, data->packets_received, data->packet_loss);
                } else {
                    plugin_ctx->log_message(LOG_LVL_WARN, "Failed to match all packet patterns");
                }
            }
        }
    } else {
        plugin_ctx->log_message(LOG_LVL_ERROR, "Failed to find statistics section in ping output");
    }
    
    // Extract RTT stats
    const char *rtt_line = strstr(output, "rtt min/avg/max");
    if (!rtt_line) {
        // Try alternative format
        rtt_line = strstr(output, "min/avg/max");
    }
    
    if (rtt_line) {
        // Find the equals sign or a sequence of numeric values
        char *values_start = strstr(rtt_line, "=");
        if (values_start) {
            // Skip the equals sign and any whitespace
            values_start++;
            while (*values_start == ' ' || *values_start == '\t')
                values_start++;
        } else {
            // No equals sign, look for the first numeric value after min/avg/max
            values_start = (char*)rtt_line + 11; // Skip "rtt min/avg"
            while (*values_start != '\0' && !(*values_start >= '0' && *values_start <= '9') && *values_start != '.')
                values_start++;
        }
        
        // Parse min/avg/max RTT
        float min = 0, avg = 0, max = 0;
        int count = sscanf(values_start, "%f/%f/%f", &min, &avg, &max);
        if (count == 3) {
            data->min_rtt = min;
            data->avg_rtt = avg;
            data->max_rtt = max;
            plugin_ctx->log_message(LOG_LVL_DEBUG, "Parsed RTT values: min=%.3f, avg=%.3f, max=%.3f", 
                                  data->min_rtt, data->avg_rtt, data->max_rtt);
        } else {
            plugin_ctx->log_message(LOG_LVL_WARN, "Failed to parse RTT values, only got %d values", count);
        }
    } else {
        plugin_ctx->log_message(LOG_LVL_WARN, "Could not find RTT statistics in ping output");
    }
    
    // Consider the test successful if we have:
    // 1. Both packet counts, or
    // 2. Valid RTT data
    return ((data->packets_sent > 0 && data->packets_received > 0) || 
            (data->min_rtt > 0 && data->avg_rtt > 0 && data->max_rtt > 0)) ? 0 : -1;
}

// Plugin API implementation

PLUGIN_EXPORT const plugin_info_t* plugin_get_info(void)
{
    return &PLUGIN_INFO;
}

PLUGIN_EXPORT const plugin_capabilities_t* plugin_get_capabilities(void)
{
    return &PLUGIN_CAPABILITIES;
}

PLUGIN_EXPORT int plugin_init(plugin_context_t *context)
{
    if (!context) return -1;
    
    // Store context for later use
    plugin_ctx = context;
    plugin_ctx->log_message(0, "Ping plugin initialized");
    return 0;
}

PLUGIN_EXPORT int plugin_cleanup(plugin_context_t *context)
{
    if (!context) return -1;
    
    plugin_ctx->log_message(0, "Ping plugin cleaned up");
    plugin_ctx = NULL;
    return 0;
}

PLUGIN_EXPORT int plugin_execute(const plugin_context_t *context, 
                               cJSON *input_params, 
                               test_result_info_t *result)
{
    if (!context || !result) return -1;
    
    // Initialize result structure
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, "ping_test", sizeof(result->test_id) - 1);
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_ERROR;
    
    // Extract target hostname from input parameters
    const char *target = "8.8.8.8";  // Default target
    int count = 5;                   // Default count
    int size = 64;                   // Default size
    float interval = 1.0f;           // Default interval in seconds
    bool ipv6 = false;               // Default to IPv4
    
    if (input_params) {
        cJSON *host = cJSON_GetObjectItem(input_params, "host");
        if (host && cJSON_IsString(host) && strlen(host->valuestring) > 0) {
            target = host->valuestring;
        }
        
        cJSON *count_json = cJSON_GetObjectItem(input_params, "count");
        if (count_json && cJSON_IsNumber(count_json)) {
            count = count_json->valueint;
            if (count < 1) count = 1;
            if (count > 20) count = 20;  // Limit max count
        }
        
        cJSON *size_json = cJSON_GetObjectItem(input_params, "size");
        if (size_json && cJSON_IsNumber(size_json)) {
            size = size_json->valueint;
            if (size < 16) size = 16;
            if (size > 1472) size = 1472;  // Limit max size
        }
        
        cJSON *interval_json = cJSON_GetObjectItem(input_params, "interval");
        if (interval_json && cJSON_IsNumber(interval_json)) {
            interval = interval_json->valuedouble;
            if (interval < 0.2f) interval = 0.2f;
            if (interval > 5.0f) interval = 5.0f;  // Limit interval range
        }
        
        cJSON *ipv6_json = cJSON_GetObjectItem(input_params, "ipv6");
        if (ipv6_json && cJSON_IsBool(ipv6_json)) {
            ipv6 = cJSON_IsTrue(ipv6_json);
        }
    }
    
    // Validate target
    if (!target || strlen(target) == 0) {
        context->log_message(2, "Empty target for ping test");
        snprintf(result->result_details, sizeof(result->result_details), 
                "Invalid target: empty string");
        return -1;
    }
    
    // Build ping command
    char ping_cmd[512];
    const char *ping_cmd_base = ipv6 ? "ping6" : "ping";
    
    snprintf(ping_cmd, sizeof(ping_cmd), 
             "%s -c %d -s %d -i %.1f %s", 
             ping_cmd_base, count, size, interval, target);
    
    context->log_message(0, "Executing ping command: %s", ping_cmd);
    
    // Execute ping command
    FILE *pipe = popen(ping_cmd, "r");
    if (!pipe) {
        context->log_message(LOG_LVL_ERROR, "Failed to execute ping command: %s", ping_cmd);
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Failed to execute ping command: %s", strerror(errno));
        return -1;
    }
    
    // Read ping output
    char buffer[4096] = {0};
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, pipe);
    buffer[bytes_read] = '\0';
    
    // Close pipe and check exit code
    int exit_code = pclose(pipe);
    context->log_message(LOG_LVL_DEBUG, "Ping command exited with status %d", exit_code);
    
    // Process ping output
    if (bytes_read > 0) {
        // Parse ping results - print first 100 chars of output for debugging
        context->log_message(LOG_LVL_DEBUG, "Ping output preview: %.100s...", buffer);
        
        if (plugin_parse_ping_result(buffer, &result->data.ping) == 0) {
            result->status = TEST_RESULT_SUCCESS;
            
            snprintf(result->result_details, sizeof(result->result_details), 
                    "Ping to %s completed. Packets: %d/%d, Loss: %.1f%%, RTT min/avg/max: %.3f/%.3f/%.3f ms", 
                    target, 
                    result->data.ping.packets_received, 
                    result->data.ping.packets_sent,
                    result->data.ping.packet_loss, 
                    result->data.ping.min_rtt, 
                    result->data.ping.avg_rtt, 
                    result->data.ping.max_rtt);
        } else {
            // Check if ping command reported success even though parsing failed
            if (exit_code == 0) {
                context->log_message(LOG_LVL_WARN, "Ping command succeeded but parsing failed, treating as partial success");
                result->status = TEST_RESULT_SUCCESS;
                snprintf(result->result_details, sizeof(result->result_details), 
                        "Ping to %s completed but results couldn't be fully parsed.", target);
            } else {
                result->status = TEST_RESULT_FAILED;
                snprintf(result->result_details, sizeof(result->result_details), 
                        "Ping to %s failed. All packets lost.", target);
            }
        }
    } else {
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "No output from ping command");
    }
    
    // Set timing information (simplified)
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    // Execution already completed, this is just for demonstration
    gettimeofday(&end_time, NULL);
    result->execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0f + 
                            (end_time.tv_usec - start_time.tv_usec) / 1000.0f;
    
    context->log_message(0, "Ping test completed with status: %d", result->status);
    return 0;
}
