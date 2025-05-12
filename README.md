# README - `testing_device` System

## Table of Contents

- [Introduction](#introduction)
- [How the System Works](#how-the-system-works)
  - [Overview Process](#overview-process)
  - [Test Case File Structure](#test-case-file-structure)
- [Guide to Adding New Test Cases](#guide-to-adding-new-test-cases)
  - [Create JSON Test Case File](#create-json-test-case-file)
  - [Write Handler for New Service](#write-handler-for-new-service)
  - [Register Service in the System](#register-service-in-the-system)
  - [Update Makefile](#update-makefile)
  - [Build and Test](#build-and-test)
- [Notes and Tips](#notes-and-tips)
- [Conclusion](#conclusion)

---

## Introduction

The `testing_device` system is an application that automatically executes test cases defined in JSON files. The system is designed to:

- Monitor the `config` directory to detect new test case files (JSON files).
- Parse JSON files to get the list of test cases.
- Execute test cases according to the specified service.
- Write results to the `result` directory and move processed files to the `processed` directory.

The system runs as a systemd service (`testing_device.service`), automatically starting when the system boots, ensuring continuous operation.

---

## How the System Works

### Overview Process

The `testing_device` system operates in the following steps:

1. **Monitor the `config` directory**:
   - Use `inotify` to detect new JSON files in the `config` directory.
   - When a JSON file is detected (like `ping.json`), the system adds the file path to a queue for sequential processing.

2. **Get file from queue**:
   - The system gets files from the queue in order of detection (first-come, first-served).
   - Calls the processing function to read and execute the JSON file.

3. **Read and parse JSON file**:
   - Read the entire JSON file content into memory.
   - Parse the JSON content to get the test case list (`test_cases` array).
   - Each test case contains: `service` (required), `action` (optional), and `params` (optional).

4. **Execute test case**:
   - For each test case, the system looks up the handler corresponding to the `service`.
   - Calls the handler to execute the test case (e.g., ping host, check network speed).
   - Records execution results in a JSON array (`result_array`).

5. **Write results and move file**:
   - After executing all test cases, results are written to a JSON file in the `result` directory (e.g., `result/ping_20250506120000_result.json`).
   - The original JSON file is moved to the `processed` directory (e.g., `processed/ping.json`) to avoid reprocessing.

### Test Case File Structure

The test case file is a JSON file containing a list of test cases. The basic structure is as follows:

- **`"test_cases"`**: An array containing the list of test cases.
- **Each test case**:
  - `service` (required): Service name (e.g., `"ping"`).
  - `action` (optional): Specific action (default is `"default"` if not specified).
  - `params` (optional): Parameters for the test case, as a JSON object (e.g., `{"host": "youtube.com"}`).

**Example `ping.json` file**:

```json
{
    "test_cases": [
        {
            "service": "ping",
            "params": {
                "host": "youtube.com"
            }
        },
        {
            "service": "ping",
            "params": {
                "host": "facebook.com"
            }
        }
    ]
}
```

---

## Guide to Adding New Test Cases

If you want to add a new test case (e.g., create a WAN connection with the `wan` service), follow these steps:

### Create JSON Test Case File

1. **Create JSON file**:
   - Create a new JSON file in the `config` directory, e.g., `config/wan.json`.
   - The file content has the following structure:

     ```json
     {
         "test_cases": [
             {
                 "service": "wan",
                 "action": "create",
                 "params": {
                     "type": "pppoe",
                     "username": "test_user",
                     "password": "test_pass"
                 }
             },
             {
                 "service": "wan",
                 "action": "create",
                 "params": {
                     "type": "dhcp",
                     "interface": "eth0"
                 }
             }
         ]
     }
     ```

2. **Place the file in the `config` directory**:
   - Copy the file to the `config` directory:
     ```
     cp wan.json /home/tobie/testing_device/config/
     ```
   - The system will automatically detect and process the file.

### Write Handler for New Service

If the `wan` service is not yet supported, you need to write a handler for it:

1. **Create header file**:
   - Create the file `include/wan.h`.
   - Declare the handler prototype:

     ```c
     void execute_wan(TestCase *test_case, const char *filepath, int index, cJSON *result_array);
     ```

2. **Create source file**:
   - Create the file `src/test/wan.c`.
   - Implement the `execute_wan` function:
     - Parse parameters from `params` like `type`, `username`, `password`, `interface`.
     - Check the test case action:
       If `action` is `"create"`, create a WAN connection according to the specified type (PPPoE or DHCP).
     - Write the result to `result_array` (pass if created successfully, fail if not).

### Register Service in the System

1. **Open the `action_registry.c` file**:
   - This file contains the `init_action_dispatch` function, where services are registered.

2. **Add registration command**:
   - In the `init_action_dispatch` function, add the following line:

     ```c
      register_service("wan", execute_wan);
     ```

   - This line maps the `service`: `"wan"` to the `execute_wan` handler.

### Update Makefile

1. **Open the `Makefile`**:
   - Add the new file to the `SOURCES` and `OBJECTS` lists.

2. **Update the lists**:
   - Add `$(TEST_DIR)/wan.c` to `SOURCES`.
   - Add `$(BIN_DIR)/wan.o` to `OBJECTS`.
   - Add `$(INCLUDE_DIR)/wan.h` to `HEADERS`.
   - Example:

     ```makefile
     SOURCES = \
         $(CORE_DIR)/action.c \
         $(CORE_DIR)/action_registry.c \
         $(CORE_DIR)/config_watcher.c \
         $(CORE_DIR)/file_process.c \
         $(CORE_DIR)/log.c \
         $(CORE_DIR)/main.c \
         $(CORE_DIR)/parser.c \
         $(CORE_DIR)/test_case_handler.c \
         $(TEST_DIR)/ping.c \
         $(TEST_DIR)/wan.c

     OBJECTS = \
         $(BIN_DIR)/action.o \
         $(BIN_DIR)/action_registry.o \
         $(BIN_DIR)/config_watcher.o \
         $(BIN_DIR)/file_process.o \
         $(BIN_DIR)/log.o \
         $(BIN_DIR)/main.o \
         $(BIN_DIR)/parser.o \
         $(BIN_DIR)/test_case_handler.o \
         $(BIN_DIR)/ping.o \
         $(BIN_DIR)/wan.o

     HEADERS = \
         $(INCLUDE_DIR)/action.h \
         $(INCLUDE_DIR)/action_registry.h \
         $(INCLUDE_DIR)/config_watcher.h \
         $(INCLUDE_DIR)/file_process.h \
         $(INCLUDE_DIR)/log.h \
         $(INCLUDE_DIR)/parser.h \
         $(INCLUDE_DIR)/ping.h \
         $(INCLUDE_DIR)/test_case_handler.h \
         $(INCLUDE_DIR)/types.h \
         $(INCLUDE_DIR)/wan.h
     ```

### Build and Test

1. **Rebuild the system**:
   - Run the command to clean and rebuild:

     ```
     make clean
     make
     ```

2. **Check the results**:
   - Ensure the `testing_device.service` service is running:

     ```
     sudo systemctl status testing_device.service
     ```

     - If it's not running, start the service:

       ```
       sudo systemctl start testing_device.service
       ```

   - Place the `wan.json` file in the `config` directory.
   - Check the result file in the `result` directory (e.g., `result/wan.json_20250506120000_result.json`).
   - Check the log to see the execution process (typically in `/var/log/testing_device.log` or stdout/stderr).

---

## Notes and Tips

- **Ensure unique JSON filenames**:
  - When placing JSON files in `config`, make sure the filename is unique (e.g., add a timestamp like `ping_20250506120000.json`) to avoid overwriting in the `processed` directory.

- **Check the logs**:
  - Logs written by the system are very useful for debugging. Check the logs to see if files are being detected, processed, and results are being written correctly.

- **Processing time**:
  - The system processes JSON files sequentially (according to the queue). If a file contains many test cases or test cases take a long time (like pinging a host), subsequent files will have to wait. Consider the number of test cases in each file.

- **Check supported services**:
  - Before adding a new test case, check if the `service` is already supported (look in `init_action_dispatch` of `action_registry.c`). If not, you need to write a new handler.

- **Backup files before running**:
  - JSON files will be moved from `config` to `processed` after processing. Back up the file if you need to use it again.

---

## Conclusion

The `testing_device` system provides a flexible way to execute automated test cases. Users can easily add new test cases by creating JSON files and writing handlers if needed. If you encounter issues or need additional support, check the logs and contact the development team.