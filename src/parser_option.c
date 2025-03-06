/**
 * @file parser_option.c
 * @brief Implementation of command line options parser
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <getopt.h>
 #include <unistd.h>
 
 #include "../include/parser_option.h"
 #include "../include/log.h"
 #include "../include/packet_process.h"
 #include "../include/parser_data.h"
 #include "../include/get_data.h"
 
 // Default configuration values
 #define DEFAULT_THREAD_COUNT 4
 #define DEFAULT_QUEUE_SIZE 10000
 #define DEFAULT_BATCH_SIZE 1000
 #define DEFAULT_LOG_LEVEL LOG_LVL_INFO
 #define DEFAULT_PROGRESS_INTERVAL 100000
 #define DEFAULT_PC_PORT 22
 
 // Static progress interval
 static int global_progress_interval = DEFAULT_PROGRESS_INTERVAL;
 
 int init_default_config(system_config_t* config) {
     if (!config) {
         return -1;
     }
     
     // Initialize with default values
     memset(config, 0, sizeof(system_config_t));
     config->thread_count = DEFAULT_THREAD_COUNT;
     config->queue_size = DEFAULT_QUEUE_SIZE;
     config->batch_size = DEFAULT_BATCH_SIZE;
     config->log_level = DEFAULT_LOG_LEVEL;
     config->progress_interval = DEFAULT_PROGRESS_INTERVAL;
     config->network_type = NETWORK_TYPE_LAN;  // Default to LAN
     config->compress_results = true;
     config->pc_port = DEFAULT_PC_PORT;
     
     return 0;
 }
 
 void print_help(const char* program_name) {
     printf("Usage: %s [OPTIONS]\n\n", program_name);
     printf("Options:\n");
     printf("  -h, --help                    Display this help message\n");
     printf("  -t, --threads NUM             Set number of worker threads (default: %d)\n", DEFAULT_THREAD_COUNT);
     printf("  -q, --queue-size NUM          Set task queue size (default: %d)\n", DEFAULT_QUEUE_SIZE);
     printf("  -b, --batch-size NUM          Set batch processing size (default: %d)\n", DEFAULT_BATCH_SIZE);
     printf("  -l, --log-level LEVEL         Set log level (NONE, ERROR, WARN, INFO, DEBUG) (default: INFO)\n");
     printf("  -p, --progress-interval NUM   Set progress reporting interval (default: %d test cases)\n", DEFAULT_PROGRESS_INTERVAL);
     printf("  -i, --input FILE              Specify input JSON test cases file\n");
     printf("  -o, --output-dir DIR          Specify output directory for results and reports\n");
     printf("  -n, --network-type TYPE       Set network type to filter for (LAN, WAN) (default: LAN)\n");
     printf("  -c, --compress                Compress results before sending (default: enabled)\n");
     printf("  -s, --pc-host HOST            Specify PC App host address\n");
     printf("  -P, --pc-port PORT            Specify PC App SSH port (default: %d)\n", DEFAULT_PC_PORT);
     printf("  -u, --pc-username USER        Specify PC App SSH username\n");
     printf("  -k, --pc-key-path FILE        Specify path to SSH private key\n");
     printf("  -r, --pc-remote-dir DIR       Specify remote directory on PC App\n");
     printf("  -C, --config-file FILE        Load configuration from file\n");
     printf("\n");
 }
 
 int parse_options(int argc, char* argv[], system_config_t* config) {
     if (!argv || !config) {
         return -1;
     }
     
     // Initialize with default values first
     init_default_config(config);
     
     // Define long options
     static struct option long_options[] = {
         {"help",             no_argument,       0, 'h'},
         {"threads",          required_argument, 0, 't'},
         {"queue-size",       required_argument, 0, 'q'},
         {"batch-size",       required_argument, 0, 'b'},
         {"log-level",        required_argument, 0, 'l'},
         {"progress-interval",required_argument, 0, 'p'},
         {"input",            required_argument, 0, 'i'},
         {"output-dir",       required_argument, 0, 'o'},
         {"network-type",     required_argument, 0, 'n'},
         {"compress",         no_argument,       0, 'c'},
         {"pc-host",          required_argument, 0, 's'},
         {"pc-port",          required_argument, 0, 'P'},
         {"pc-username",      required_argument, 0, 'u'},
         {"pc-key-path",      required_argument, 0, 'k'},
         {"pc-remote-dir",    required_argument, 0, 'r'},
         {"config-file",      required_argument, 0, 'C'},
         {0, 0, 0, 0}
     };
     
     // Define short options
     const char* short_options = "ht:q:b:l:p:i:o:n:cs:P:u:k:r:C:";
     
     // Process options
     int option;
     int option_index = 0;
     
     while ((option = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
         switch (option) {
             case 'h':
                 print_help(argv[0]);
                 exit(0);
                 break;
                 
             case 't':
                 config->thread_count = atoi(optarg);
                 break;
                 
             case 'q':
                 config->queue_size = atoi(optarg);
                 break;
                 
             case 'b':
                 config->batch_size = atoi(optarg);
                 break;
                 
             case 'l':
                 if (strcmp(optarg, "NONE") == 0) {
                     config->log_level = LOG_LVL_NONE;
                 } else if (strcmp(optarg, "ERROR") == 0) {
                     config->log_level = LOG_LVL_ERROR;
                 } else if (strcmp(optarg, "WARN") == 0) {
                     config->log_level = LOG_LVL_WARN;
                 } else if (strcmp(optarg, "INFO") == 0) {
                     config->log_level = LOG_LVL_INFO;
                 } else if (strcmp(optarg, "DEBUG") == 0) {
                     config->log_level = LOG_LVL_DEBUG;
                 } else {
                     fprintf(stderr, "Invalid log level: %s\n", optarg);
                     print_help(argv[0]);
                     return -1;
                 }
                 break;
                 
             case 'p':
                 config->progress_interval = atoi(optarg);
                 break;
                 
             case 'i':
                 strncpy(config->input_file, optarg, sizeof(config->input_file) - 1);
                 break;
                 
             case 'o':
                 strncpy(config->output_dir, optarg, sizeof(config->output_dir) - 1);
                 break;
                 
             case 'n':
                 if (strcmp(optarg, "LAN") == 0) {
                     config->network_type = NETWORK_TYPE_LAN;
                 } else if (strcmp(optarg, "WAN") == 0) {
                     config->network_type = NETWORK_TYPE_WAN;
                 } else {
                     fprintf(stderr, "Invalid network type: %s\n", optarg);
                     print_help(argv[0]);
                     return -1;
                 }
                 break;
                 
             case 'c':
                 config->compress_results = true;
                 break;
                 
             case 's':
                 strncpy(config->pc_host, optarg, sizeof(config->pc_host) - 1);
                 break;
                 
             case 'P':
                 config->pc_port = atoi(optarg);
                 break;
                 
             case 'u':
                 strncpy(config->pc_username, optarg, sizeof(config->pc_username) - 1);
                 break;
                 
             case 'k':
                 strncpy(config->pc_key_path, optarg, sizeof(config->pc_key_path) - 1);
                 break;
                 
             case 'r':
                 strncpy(config->pc_remote_dir, optarg, sizeof(config->pc_remote_dir) - 1);
                 break;
                 
             case 'C':
                 // Parse configuration file
                 if (parse_config_file(optarg, config) != 0) {
                     fprintf(stderr, "Failed to parse configuration file: %s\n", optarg);
                     return -1;
                 }
                 break;
                 
             case '?':
                 // getopt_long already printed an error message
                 print_help(argv[0]);
                 return -1;
                 
             default:
                 fprintf(stderr, "Unknown option: %c\n", option);
                 print_help(argv[0]);
                 return -1;
         }
     }
     
     return 0;
 }
 
 int parse_config_file(const char* config_file, system_config_t* config) {
     if (!config_file || !config) {
         return -1;
     }
     
     FILE* fp = fopen(config_file, "r");
     if (!fp) {
         fprintf(stderr, "Failed to open configuration file: %s\n", config_file);
         return -1;
     }
     
     char line[512];
     char key[64], value[256];
     
     while (fgets(line, sizeof(line), fp) != NULL) {
         // Skip comments and empty lines
         if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
             continue;
         }
         
         // Parse key=value pairs
         if (sscanf(line, "%63[^=]=%255[^\n]", key, value) == 2) {
             // Trim whitespace from key and value
             char* k = key;
             while (*k && (*k == ' ' || *k == '\t')) k++;
             
             char* v = value;
             while (*v && (*v == ' ' || *v == '\t')) v++;
             
             // Remove trailing whitespace from value
             char* end = v + strlen(v) - 1;
             while (end > v && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
                 *end = '\0';
                 end--;
             }
             
             // Process configuration options
             if (strcmp(k, "thread_count") == 0) {
                 config->thread_count = atoi(v);
             }
             else if (strcmp(k, "queue_size") == 0) {
                 config->queue_size = atoi(v);
             }
             else if (strcmp(k, "batch_size") == 0) {
                 config->batch_size = atoi(v);
             }
             else if (strcmp(k, "log_level") == 0) {
                 if (strcmp(v, "NONE") == 0) {
                     config->log_level = LOG_LVL_NONE;
                 } else if (strcmp(v, "ERROR") == 0) {
                     config->log_level = LOG_LVL_ERROR;
                 } else if (strcmp(v, "WARN") == 0) {
                     config->log_level = LOG_LVL_WARN;
                 } else if (strcmp(v, "INFO") == 0) {
                     config->log_level = LOG_LVL_INFO;
                 } else if (strcmp(v, "DEBUG") == 0) {
                     config->log_level = LOG_LVL_DEBUG;
                 }
             }
             else if (strcmp(k, "progress_interval") == 0) {
                 config->progress_interval = atoi(v);
             }
             else if (strcmp(k, "input_file") == 0) {
                 strncpy(config->input_file, v, sizeof(config->input_file) - 1);
             }
             else if (strcmp(k, "output_dir") == 0) {
                 strncpy(config->output_dir, v, sizeof(config->output_dir) - 1);
             }
             else if (strcmp(k, "network_type") == 0) {
                 if (strcmp(v, "LAN") == 0) {
                     config->network_type = NETWORK_TYPE_LAN;
                 } else if (strcmp(v, "WAN") == 0) {
                     config->network_type = NETWORK_TYPE_WAN;
                 }
             }
             else if (strcmp(k, "compress_results") == 0) {
                 config->compress_results = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);
             }
             else if (strcmp(k, "pc_host") == 0) {
                 strncpy(config->pc_host, v, sizeof(config->pc_host) - 1);
             }
             else if (strcmp(k, "pc_port") == 0) {
                 config->pc_port = atoi(v);
             }
             else if (strcmp(k, "pc_username") == 0) {
                 strncpy(config->pc_username, v, sizeof(config->pc_username) - 1);
             }
             else if (strcmp(k, "pc_key_path") == 0) {
                 strncpy(config->pc_key_path, v, sizeof(config->pc_key_path) - 1);
             }
             else if (strcmp(k, "pc_remote_dir") == 0) {
                 strncpy(config->pc_remote_dir, v, sizeof(config->pc_remote_dir) - 1);
             }
         }
     }
     
     fclose(fp);
     return 0;
 }
 
 int set_progress_interval(int interval) {
     if (interval <= 0) {
         return -1;
     }
     
     global_progress_interval = interval;
     return 0;
 }
 
 int apply_system_config(const system_config_t* config) {
     if (!config) {
         return -1;
     }
     
     // Initialize logging system
     char log_file[256];
     if (strlen(config->output_dir) > 0) {
         snprintf(log_file, sizeof(log_file), "%s/logs/log.txt", config->output_dir);
     } else {
         strcpy(log_file, "logs/log.txt");
     }
     
     if (log_init(log_file, config->log_level) != 0) {
         fprintf(stderr, "Failed to initialize logging system\n");
         return -1;
     }
     
     LOG_INFO(LOG_CAT_GENERAL, "Applying system configuration");
     
     // Set global progress interval
     set_progress_interval(config->progress_interval);
     
     // Initialize thread pool
     if (init_thread_pool(config->thread_count, config->queue_size) != 0) {
         LOG_ERROR(LOG_CAT_GENERAL, "Failed to initialize thread pool");
         return -1;
     }
     
     // Initialize SSH config if PC App connection parameters are provided
     if (strlen(config->pc_host) > 0 && strlen(config->pc_username) > 0) {
         if (init_ssh_config(config->pc_host, config->pc_port, 
                           config->pc_username, NULL, config->pc_key_path, 
                           config->pc_remote_dir) != 0) {
             LOG_ERROR(LOG_CAT_GENERAL, "Failed to initialize SSH configuration");
             return -1;
         }
         
         // Test SSH connection
         if (test_ssh_connection() != 0) {
             LOG_ERROR(LOG_CAT_GENERAL, "Failed to connect to PC App via SSH");
             return -1;
         }
     }
     
     LOG_INFO(LOG_CAT_GENERAL, "System configuration applied successfully");
     return 0;
 }