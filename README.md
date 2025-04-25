# Device Testing System

## Overview

**Device Testing System** is a tool designed to perform diagnostic tests on network devices.

This document provides basic instructions for building, running the system, and reading log results.

## Requirements

Before using the system, make sure you have installed the following tools on your Linux operating system:

- **GCC Compiler**: To build the project.

  ```
  sudo apt-get install gcc
  ```
- **Make**: To build the project using Makefile.

  ```
  sudo apt-get install make
  ```
- **cJSON Library**: To process JSON configuration files.

  ```
  sudo apt-get install libcjson-dev
  ```
- **speedtest-cli**: Required for the `speedtest` test to measure network speed (optional if not using `speedtest`).

  ```
  sudo apt-get install speedtest-cli
  ```

## Directory Structure

- `config/`: Contains test configuration files in JSON format (`ping.json`, `speedtest.json`, etc.).
- `src/`: Contains source code files.
- `include/`: Contains header files.
- `build/`: Directory containing the executable file after compilation (`testing_device`).
- `bin/`: Temporary directory to store object files during compilation.

## Compiling the Project

1. Navigate to the project directory:

   ```
   cd testing_device
   ```

2. Compile the project using Makefile:

   ```
   make
   ```

   - This command will compile the source code and create the executable file `build/testing_device`.

3. If you need to clean up the compiled files:

   ```
   make clean
   ```

## Running Tests

### Running Specific Tests

You can run specific tests by providing paths to JSON configuration files via command line parameters.

**Example**:

```
./build/testing_device config/ping.json config/speedtest.json
```

- This command runs the `ping` test (from `ping.json`) and then the `speedtest` test (from `speedtest.json`).

## Available Tests

### 1. `ping`

- **Purpose**: Check network connectivity by sending ICMP packets to a specified host.
- **Configuration File**: `config/ping.json`
- **Input Parameters**:
  - `host`: Target host to ping (e.g., `"google.com"`).
  - `plugin`: Optional plugin name (e.g., `"Debug Ping Plugin"`).
- **Output Results**:
  - `Ping.Status`: 0 (success) or 1 (failure).
  - `Ping.host`: Target host.
  - `Ping.hostAddress`: IP address of the target host.
  - `Ping.successCount`: Number of successful packets received.
  - `Ping.failureCount`: Number of packets lost.
  - `Ping.averageResponseTime`: Average response time (ms).
  - `Ping.minimumResponseTime`: Minimum response time (ms).
  - `Ping.maximumResponseTime`: Maximum response time (ms).
  - `Ping.jitter`: Network jitter (ms).
  - `Ping.packetLoss`: Packet loss rate (%).

### 2. `speedtest`

- **Purpose**: Measure network speed (download, upload, and latency).
- **Configuration File**: `config/speedtest.json`
- **Input Parameters** (optional):
  - `server`: Speedtest server name (e.g., `"speedtest.net"`). If not specified, the system will use the nearest server.
- **Output Results**:
  - `Speedtest.Status`: 0 (success) or 1 (failure).
  - `Server`: Name of the server used for testing.
  - `Server Location`: City and country of the server.
  - `ISP`: Internet service provider used.
  - `Download`: Download speed (Mbps).
  - `Upload`: Upload speed (Mbps).
  - `Latency`: Network latency (ms).

## Reading Log Results

The system generates detailed logs to help you understand the execution process and test results. Logs are written to the `application.log` file (if configured in `config.json`) and also displayed on the console.

### Log Structure

- **Timestamp**: Each log line starts with a timestamp (e.g., `[2025-04-22 11:00:54]`).
- **Log Level**: Indicates the importance level (`DEBUG`, `INFO`, `WARN`, `ERROR`).
- **Message**: Describes the action or result.

### Key Sections to Note

1. **Loading Tests**:

   - Lines like `Processing test configuration file: config/ping.json` indicate the system is loading a test.
   - `Successfully read XXX bytes from file` and `Successfully parsed X test cases` confirm the file was loaded and parsed successfully.

2. **Executing Tests**:

   - `Executing action: ping (type: diagnostic)`: Shows the action being executed and its type.
   - `Executing ping test`: Indicates the handler (`execute_ping`) is running.
   - `Param host = ...`: Displays the input parameters of the test.

3. **Test Results**:

   - For `ping`:
     - `Packets: X sent, Y received, Z% loss`: Summarizes packet transmission and reception.
     - `RTT: min=X ms, avg=Y ms, max=Z ms, jitter=W ms`: Response time metrics.
     - `Ping.Status`: Test result (0 = success, 1 = failure).
     - Other parameters: `Ping.host`, `Ping.hostAddress`, `Ping.successCount`, etc.
   - For `speedtest`:
     - `No server specified, using default server (nearest)`: Indicates the default server is used.
     - `Server ID for future reference`: ID of the server used.
     - `Speedtest completed successfully`: Includes server, ISP, and speed metrics.

4. **Errors or Warnings**:

   - Look for lines with `ERROR` or `WARN` to identify issues (e.g., `Failed to execute speedtest command` if `speedtest-cli` is not installed).

## Troubleshooting

- **Tests Not Running**:

  - Check if required tools (`speedtest-cli`) are installed.
  - Ensure JSON configuration files (`config/ping.json`, `config/speedtest.json`) are valid and exist.

- **No Results in Logs**:

  - Ensure tests have correct input parameters (e.g., `host` for `ping`).
  - Check `ERROR` lines in logs for the cause.

- **Compilation Errors**:

  - Ensure all dependencies (`gcc`, `make`, `libcjson-dev`) are installed.
  - Run `make clean` and then `make` to recompile the project.

## Extending the System

To add a new test:

1. Create a new JSON configuration file in `config/` (e.g., `new_test.json`).
2. Add the corresponding `instruction` to `config.json`.
3. Write a handler in `src/test/` (e.g., `new_test.c`) and register it in `src/action/action_registry.c`.
4. Update the `Makefile` to include the new source file.

