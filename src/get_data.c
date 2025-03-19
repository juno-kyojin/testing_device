#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "get_data.h"

#define BUFFER_SIZE 1024

// Initialize an SSH connection from Device to PC App
connection_info_t* init_connection(const char* host, int port, const char* username, const char* password) {
    ssh_session session = ssh_new();
    if (session == NULL) {
        fprintf(stderr, "Failed to create SSH session\n");
        return NULL;
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, host);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_USER, username);

    if (ssh_connect(session) != SSH_OK) {
        fprintf(stderr, "SSH connection failed: %s\n", ssh_get_error(session));
        ssh_free(session);
        return NULL;
    }

    if (ssh_userauth_password(session, NULL, password) != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        return NULL;
    }

    connection_info_t* conn = (connection_info_t*) malloc(sizeof(connection_info_t));
    if (conn == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        ssh_disconnect(session);
        ssh_free(session);
        return NULL;
    }

    strncpy(conn->host, host, sizeof(conn->host) - 1);
    conn->host[sizeof(conn->host) - 1] = '\0';
    conn->port = port;
    strncpy(conn->username, username, sizeof(conn->username) - 1);
    conn->username[sizeof(conn->username) - 1] = '\0';
    strncpy(conn->password, password, sizeof(conn->password) - 1);
    conn->password[sizeof(conn->password) - 1] = '\0';
    conn->session = session;

    return conn;
}

// Receive network configuration from PC App
int receive_network_config_from_pc(connection_info_t* conn, network_config_t* config) {
    if (conn == NULL || config == NULL) return -1;

    ssh_channel channel = ssh_channel_new(conn->session);
    if (channel == NULL) {
        fprintf(stderr, "Failed to create SSH channel\n");
        return -1;
    }

    if (ssh_channel_open_session(channel) != SSH_OK) {
        fprintf(stderr, "Failed to open channel: %s\n", ssh_get_error(conn->session));
        ssh_channel_free(channel);
        return -1;
    }

    // Request network config (assumes PC App has a command "send_network_config")
    if (ssh_channel_request_exec(channel, "send_network_config") != SSH_OK) {
        fprintf(stderr, "Failed to execute command: %s\n", ssh_get_error(conn->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int nbytes;
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[nbytes] = '\0';
        // Placeholder: Parse buffer into config (e.g., JSON parsing with a library like cJSON)
        printf("Received network config: %s\n", buffer);
        // Actual parsing depends on the data format; implement as needed
    }

    if (nbytes < 0) {
        fprintf(stderr, "Error reading channel: %s\n", ssh_get_error(conn->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return -1;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return 0;
}

// Receive test cases from PC App and save to a file
int receive_test_cases_from_pc(connection_info_t* conn, const char* output_file) {
    if (conn == NULL || output_file == NULL) return -1;

    sftp_session sftp = sftp_new(conn->session);
    if (sftp == NULL) {
        fprintf(stderr, "Failed to create SFTP session\n");
        return -1;
    }

    if (sftp_init(sftp) != SSH_OK) {
        fprintf(stderr, "SFTP initialization failed: %s\n", ssh_get_error(conn->session));
        sftp_free(sftp);
        return -1;
    }

    sftp_file file = sftp_open(sftp, output_file, O_WRONLY | O_CREAT, S_IRWXU);
    if (file == NULL) {
        fprintf(stderr, "Failed to open output file: %s\n", ssh_get_error(conn->session));
        sftp_free(sftp);
        return -1;
    }

    ssh_channel channel = ssh_channel_new(conn->session);
    if (channel == NULL) {
        sftp_close(file);
        sftp_free(sftp);
        return -1;
    }

    if (ssh_channel_open_session(channel) != SSH_OK) {
        fprintf(stderr, "Failed to open channel: %s\n", ssh_get_error(conn->session));
        ssh_channel_free(channel);
        sftp_close(file);
        sftp_free(sftp);
        return -1;
    }

    // Assumes PC App has a command "send_test_cases" to send test case data
    if (ssh_channel_request_exec(channel, "send_test_cases") != SSH_OK) {
        fprintf(stderr, "Failed to execute command: %s\n", ssh_get_error(conn->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        sftp_close(file);
        sftp_free(sftp);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int nbytes;
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
        if (sftp_write(file, buffer, nbytes) != nbytes) {
            fprintf(stderr, "Failed to write to file: %s\n", ssh_get_error(conn->session));
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            sftp_close(file);
            sftp_free(sftp);
            return -1;
        }
    }

    if (nbytes < 0) {
        fprintf(stderr, "Error reading channel: %s\n", ssh_get_error(conn->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        sftp_close(file);
        sftp_free(sftp);
        return -1;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    sftp_close(file);
    sftp_free(sftp);
    return 0;
}

// Send test execution progress to PC App
int send_progress_to_pc(connection_info_t* conn, int completed, int total, int success_count, int fail_count) {
    if (conn == NULL) return -1;

    ssh_channel channel = ssh_channel_new(conn->session);
    if (channel == NULL) {
        fprintf(stderr, "Failed to create SSH channel\n");
        return -1;
    }

    if (ssh_channel_open_session(channel) != SSH_OK) {
        fprintf(stderr, "Failed to open channel: %s\n", ssh_get_error(conn->session));
        ssh_channel_free(channel);
        return -1;
    }

    char progress_str[BUFFER_SIZE];
    snprintf(progress_str, sizeof(progress_str), "Progress: %d/%d, Success: %d, Fail: %d",
             completed, total, success_count, fail_count);

    if (ssh_channel_write(channel, progress_str, strlen(progress_str)) < 0) {
        fprintf(stderr, "Failed to write progress: %s\n", ssh_get_error(conn->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return -1;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return 0;
}

// Send test results to PC App
int send_results_to_pc(connection_info_t* conn, test_result_info_t* results, int count) {
    if (conn == NULL || results == NULL || count <= 0) return -1;

    ssh_channel channel = ssh_channel_new(conn->session);
    if (channel == NULL) {
        fprintf(stderr, "Failed to create SSH channel\n");
        return -1;
    }

    if (ssh_channel_open_session(channel) != SSH_OK) {
        fprintf(stderr, "Failed to open channel: %s\n", ssh_get_error(conn->session));
        ssh_channel_free(channel);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        char result_str[BUFFER_SIZE];
        snprintf(result_str, sizeof(result_str), "Test ID: %s, Status: %d, Details: %s\n",
                 results[i].test_id, results[i].status, results[i].result_details);
        if (ssh_channel_write(channel, result_str, strlen(result_str)) < 0) {
            fprintf(stderr, "Failed to write result: %s\n", ssh_get_error(conn->session));
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return -1;
        }
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return 0;
}

// Send log file to PC App
int send_logs_to_pc(connection_info_t* conn, const char* log_file) {
    if (conn == NULL || log_file == NULL) return -1;

    sftp_session sftp = sftp_new(conn->session);
    if (sftp == NULL) {
        fprintf(stderr, "Failed to create SFTP session\n");
        return -1;
    }

    if (sftp_init(sftp) != SSH_OK) {
        fprintf(stderr, "SFTP initialization failed: %s\n", ssh_get_error(conn->session));
        sftp_free(sftp);
        return -1;
    }

    FILE* local_file = fopen(log_file, "r");
    if (local_file == NULL) {
        fprintf(stderr, "Failed to open local log file: %s\n", strerror(errno));
        sftp_free(sftp);
        return -1;
    }

    // Adjust the remote path as needed
    sftp_file remote_file = sftp_open(sftp, "/path/to/remote/log_file", O_WRONLY | O_CREAT, S_IRWXU);
    if (remote_file == NULL) {
        fprintf(stderr, "Failed to open remote file: %s\n", ssh_get_error(conn->session));
        fclose(local_file);
        sftp_free(sftp);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t nbytes;
    while ((nbytes = fread(buffer, 1, sizeof(buffer), local_file)) > 0) {
        ssize_t written = sftp_write(remote_file, buffer, nbytes);
        if (written != (ssize_t)nbytes) {
            fprintf(stderr, "Failed to write to remote file: %s\n", ssh_get_error(conn->session));
            sftp_close(remote_file);
            fclose(local_file);
            sftp_free(sftp);
            return -1;
        }
    }

    if (ferror(local_file)) {
        fprintf(stderr, "Error reading local file: %s\n", strerror(errno));
        sftp_close(remote_file);
        fclose(local_file);
        sftp_free(sftp);
        return -1;
    }

    sftp_close(remote_file);
    fclose(local_file);
    sftp_free(sftp);
    return 0;
}

// Send compressed results to PC App
int send_compressed_results_to_pc(connection_info_t* conn, const char* compressed_file) {
    // Reuse send_logs_to_pc logic for simplicity; adjust remote path if needed
    return send_logs_to_pc(conn, compressed_file);
}

// Close the SSH connection
int close_connection(connection_info_t* conn) {
    if (conn == NULL) return -1;

    ssh_disconnect(conn->session);
    ssh_free(conn->session);
    free(conn);
    return 0;
}