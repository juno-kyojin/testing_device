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
    .name = "speedtest_plugin",
    .version = "1.0.0",
    .description = "Internet speed test plugin using speedtest-cli",
    .author = "Testing Device Team"
};

// Plugin capabilities
static const plugin_capabilities_t PLUGIN_CAPABILITIES = {
    .action_name = "speedtest",
    .node_name = "Speedtest",
    .has_input_params = true,
    .has_custom_output = false
};

// Plugin context
static plugin_context_t *plugin_ctx = NULL;

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
    plugin_ctx->log_message(0, "Speedtest plugin initialized");
    return 0;
}

PLUGIN_EXPORT int plugin_cleanup(plugin_context_t *context)
{
    if (!context) return -1;
    
    plugin_ctx->log_message(0, "Speedtest plugin cleaned up");
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
    strncpy(result->test_id, "speedtest", sizeof(result->test_id) - 1);
    result->test_type = TEST_SPEEDTEST;
    result->status = TEST_RESULT_ERROR;
    
    // First check if command exists
    int cmd_exists = system("which speedtest-cli > /dev/null 2>&1");
    if (cmd_exists != 0) {
        // Try alternative command name (some systems use 'speedtest' instead)
        cmd_exists = system("which speedtest > /dev/null 2>&1");
        if (cmd_exists != 0) {
            context->log_message(LOG_LVL_ERROR, "speedtest-cli or speedtest command not found");
            snprintf(result->result_details, sizeof(result->result_details), 
                    "speedtest-cli not installed. Please install with: sudo apt-get install speedtest-cli or pip install speedtest-cli");
            return -1;
        }
    }
    
    // Command exists, determine which to use
    const char *speedtest_cmd = (cmd_exists == 0) ? "speedtest" : "speedtest-cli";
    
    // Extract server from input parameters
    const char *server = NULL;
    bool use_https = true;
    
    if (input_params) {
        cJSON *server_json = cJSON_GetObjectItem(input_params, "server");
        if (server_json && cJSON_IsString(server_json) && strlen(server_json->valuestring) > 0) {
            server = server_json->valuestring;
        }
        
        cJSON *https_json = cJSON_GetObjectItem(input_params, "use_https");
        if (https_json && cJSON_IsBool(https_json)) {
            use_https = cJSON_IsTrue(https_json);
        }
    }
    
    // Build speedtest command
    char command[512];
    
    if (server && atoi(server) > 0) {
        snprintf(command, sizeof(command), "%s --json --server %s%s", 
                 speedtest_cmd, server, use_https ? " --secure" : "");
        context->log_message(LOG_LVL_DEBUG, "Using server ID: %s", server);
    } else if (server && server[0]) {
        context->log_message(LOG_LVL_WARN, "Server '%s' might not be a valid server ID, attempting to use anyway", server);
        snprintf(command, sizeof(command), "%s --json --server %s%s", 
                 speedtest_cmd, server, use_https ? " --secure" : "");
    } else {
        snprintf(command, sizeof(command), "%s --json%s", 
                 speedtest_cmd, use_https ? " --secure" : "");
        context->log_message(LOG_LVL_DEBUG, "Using default server (nearest)");
    }
    
    // Record start time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    // Execute speedtest command
    context->log_message(LOG_LVL_DEBUG, "Executing command: %s", command);
    
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        context->log_message(LOG_LVL_ERROR, "Failed to open pipe for speedtest-cli");
        snprintf(result->result_details, sizeof(result->result_details), 
                "Failed to execute speedtest-cli command: %s", strerror(errno));
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    // Read speedtest output
    char json_buffer[8192] = {0};
    size_t bytes_read = fread(json_buffer, 1, sizeof(json_buffer) - 1, pipe);
    json_buffer[bytes_read] = '\0';
    
    // Close pipe and get exit code
    int exit_status = pclose(pipe);
    
    // Calculate execution time
    gettimeofday(&end_time, NULL);
    result->execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0f + 
                            (end_time.tv_usec - start_time.tv_usec) / 1000.0f;
    
    context->log_message(LOG_LVL_DEBUG, "Speedtest execution time: %.1f ms", result->execution_time);
    
    // Check execution status
    if (exit_status != 0) {
        context->log_message(LOG_LVL_ERROR, "speedtest-cli execution failed with code %d", exit_status);
        
        if (bytes_read > 0) {
            char short_error[900] = {0};
            strncpy(short_error, json_buffer, sizeof(short_error) - 1);
            
            snprintf(result->result_details, sizeof(result->result_details), 
                    "Speedtest execution failed: %s", short_error);
        } else {
            snprintf(result->result_details, sizeof(result->result_details), 
                    "Speedtest execution failed with code %d", exit_status);
        }
        
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    // Check if output is empty
    if (bytes_read == 0) {
        context->log_message(LOG_LVL_ERROR, "Empty result from speedtest");
        snprintf(result->result_details, sizeof(result->result_details), 
                "Empty result from speedtest. Try running '%s' manually", speedtest_cmd);
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    // Parse JSON result
    context->log_message(LOG_LVL_DEBUG, "Successfully read %lu bytes of JSON result", (unsigned long)bytes_read);
    cJSON *json = cJSON_Parse(json_buffer);
    if (!json) {
        context->log_message(LOG_LVL_ERROR, "Failed to parse speedtest JSON result: %s", cJSON_GetErrorPtr());
        context->log_message(LOG_LVL_DEBUG, "Raw JSON content (first 200 chars): %.200s", json_buffer);
        snprintf(result->result_details, sizeof(result->result_details), 
                "Failed to parse speedtest JSON result");
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    // Extract download, upload and ping values
    cJSON *download = cJSON_GetObjectItem(json, "download");
    cJSON *upload = cJSON_GetObjectItem(json, "upload");
    cJSON *ping = cJSON_GetObjectItem(json, "ping");
    
    if (download && cJSON_IsNumber(download)) {
        result->data.speedtest.download_speed = download->valuedouble / 1000000.0;
    }
    
    if (upload && cJSON_IsNumber(upload)) {
        result->data.speedtest.upload_speed = upload->valuedouble / 1000000.0;
    }
    
    if (ping && cJSON_IsNumber(ping)) {
        result->data.speedtest.latency = ping->valuedouble;
    }
    
    // Extract server information
    cJSON *server_obj = cJSON_GetObjectItem(json, "server");
    char server_info[256] = "Unknown server";
    if (server_obj && cJSON_IsObject(server_obj)) {
        cJSON *host = cJSON_GetObjectItem(server_obj, "host");
        cJSON *name = cJSON_GetObjectItem(server_obj, "name");
        cJSON *country = cJSON_GetObjectItem(server_obj, "country");
        cJSON *serverId = cJSON_GetObjectItem(server_obj, "id");
        
        if (host && cJSON_IsString(host) && 
            name && cJSON_IsString(name) && 
            country && cJSON_IsString(country)) {
            
            if (serverId && cJSON_IsString(serverId)) {
                snprintf(server_info, sizeof(server_info), "%s (%s, %s) [ID: %s]", 
                        name->valuestring, host->valuestring, country->valuestring, serverId->valuestring);
                
                context->log_message(LOG_LVL_DEBUG, "Server ID for future reference: %s", serverId->valuestring);
            } else {
                snprintf(server_info, sizeof(server_info), "%s (%s, %s)", 
                        name->valuestring, host->valuestring, country->valuestring);
            }
        }
    }
    
    // Create detailed result information
    snprintf(result->result_details, sizeof(result->result_details), 
            "Speedtest completed with server %s. Download: %.2f Mbps, Upload: %.2f Mbps, Latency: %.2f ms", 
            server_info,
            result->data.speedtest.download_speed,
            result->data.speedtest.upload_speed,
            result->data.speedtest.latency);
    
    // Clean up
    cJSON_Delete(json);
    
    // Set status based on results
    if (result->data.speedtest.download_speed > 0 || result->data.speedtest.upload_speed > 0) {
        result->status = TEST_RESULT_SUCCESS;
        context->log_message(LOG_LVL_DEBUG, "Speedtest completed successfully: Download=%.2f Mbps, Upload=%.2f Mbps, Latency=%.2f ms", 
                  result->data.speedtest.download_speed,
                  result->data.speedtest.upload_speed,
                  result->data.speedtest.latency);
    } else {
        result->status = TEST_RESULT_FAILED;
        context->log_message(LOG_LVL_ERROR, "Speedtest failed to get valid results");
    }
    
    return 0;
}
