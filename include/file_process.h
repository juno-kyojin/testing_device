/**
 * @file file_process.h
 * @brief Header file for file processing utilities
 *
 * This header file defines the interface for file processing utilities used in the
 * test case execution system. It provides functions for reading, writing, appending,
 * and managing files, as well as utilities for directory creation, file copying,
 * and temporary file creation. These functions are essential for handling test case
 * files and result files in the system.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see file_process.c
 */
#ifndef FILE_PROCESS_H
#define FILE_PROCESS_H

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

/**
 * @brief Read the entire contents of a file into a dynamically allocated buffer
 *
 * This function opens the specified file, reads its entire contents into a dynamically
 * allocated buffer, and returns the size of the data read. The caller is responsible
 * for freeing the allocated buffer. If the file cannot be opened or read, an error is
 * logged, and the function returns -1.
 *
 * @param file_path Path to the file to be read (e.g., "config/ping.json").
 * @param buffer Pointer to a char pointer that will be allocated to store the file contents.
 * @param size Pointer to a size_t variable where the size of the read data will be stored.
 * @return int
 *         - 0 if the file was successfully read.
 *         - -1 if an error occurred (e.g., file not found, memory allocation failed).
 */
int read_file(const char *file_path, char **buffer, size_t *size);

/**
 * @brief Write data to a file, overwriting it if it exists
 *
 * This function writes the specified data to a file, overwriting the file if it already
 * exists. If the file cannot be opened or written to, an error is logged, and the function
 * returns -1.
 *
 * @param file_path Path to the file to be written (e.g., "result/output.json").
 * @param buffer Pointer to the buffer containing the data to write.
 * @param size Size of the data to be written, in bytes.
 * @return int
 *         - 0 if the data was successfully written.
 *         - -1 if an error occurred (e.g., file cannot be opened, write error).
 */
int write_file(const char *file_path, const char *buffer, size_t size);

/**
 * @brief Append data to the end of a file
 *
 * This function appends the specified data to the end of a file. If the file does not
 * exist, it is created. If the file cannot be opened or written to, an error is logged,
 * and the function returns -1.
 *
 * @param file_path Path to the file to append to (e.g., "result/output.json").
 * @param buffer Pointer to the buffer containing the data to append.
 * @param size Size of the data to be appended, in bytes.
 * @return int
 *         - 0 if the data was successfully appended.
 *         - -1 if an error occurred (e.g., file cannot be opened, write error).
 * @note This function is currently unused in the system.
 */
int append_to_file(const char *file_path, const char *buffer, size_t size);

/**
 * @brief Check if a file exists on the file system
 *
 * This function checks whether a file exists at the specified path by attempting to
 * access it. It returns 1 if the file exists, and 0 otherwise.
 *
 * @param file_path Path to the file to check (e.g., "config/ping.json").
 * @return int
 *         - 1 if the file exists.
 *         - 0 if the file does not exist or the parameter is invalid.
 * @note This function is currently unused in the system.
 */
int file_exists(const char *file_path);

/**
 * @brief Create a new directory with specified permissions
 *
 * This function creates a new directory at the specified path with permissions set to
 * 0755 (rwxr-xr-x). If the directory already exists, the function logs a debug message
 * and returns success. If the directory cannot be created, an error is logged, and the
 * function returns -1.
 *
 * @param dir_path Path to the directory to be created (e.g., "result").
 * @return int
 *         - 0 if the directory was successfully created or already exists.
 *         - -1 if an error occurred (e.g., invalid path, permission denied).
 */
int create_directory(const char *dir_path);

/**
 * @brief Delete a file from the file system
 *
 * This function deletes the specified file from the file system. If the file cannot be
 * deleted, an error is logged, and the function returns -1.
 *
 * @param file_path Path to the file to be deleted (e.g., "config/ping.json").
 * @return int
 *         - 0 if the file was successfully deleted.
 *         - -1 if an error occurred (e.g., file not found, permission denied).
 * @note This function is currently unused in the system.
 */
int delete_file(const char *file_path);

/**
 * @brief Copy a file from a source path to a destination path
 *
 * This function copies the contents of a source file to a destination file. If the
 * destination file already exists, it will be overwritten. If the copy operation fails,
 * an error is logged, and the function returns -1.
 *
 * @param src_path Path to the source file (e.g., "config/ping.json").
 * @param dest_path Path to the destination file (e.g., "processed/ping.json").
 * @return int
 *         - 0 if the file was successfully copied.
 *         - -1 if an error occurred (e.g., source file not found, write error).
 * @note This function is currently unused in the system.
 */
int copy_file(const char *src_path, const char *dest_path);

/**
 * @brief Get the size of a file in bytes
 *
 * This function retrieves the size of the specified file in bytes. If the file does not
 * exist or an error occurs, an error is logged, and the function returns -1.
 *
 * @param file_path Path to the file to check (e.g., "config/ping.json").
 * @return long
 *         - Size of the file in bytes if successful.
 *         - -1 if an error occurred (e.g., file not found, permission denied).
 * @note This function is currently unused in the system.
 */
long get_file_size(const char *file_path);

/**
 * @brief Get the last modification time of a file
 *
 * This function retrieves the last modification time of the specified file. If the file
 * does not exist or an error occurs, an error is logged, and the function returns -1.
 *
 * @param file_path Path to the file to check (e.g., "config/ping.json").
 * @return time_t
 *         - The last modification time of the file if successful.
 *         - -1 if an error occurred (e.g., file not found, permission denied).
 * @note This function is currently unused in the system.
 */
time_t get_file_modification_time(const char *file_path);

/**
 * @brief Create a temporary file with a unique name
 *
 * This function creates a temporary file in the `/tmp` directory with a unique name
 * based on the specified prefix. The path to the temporary file is stored in the provided
 * buffer. If the temporary file cannot be created, an error is logged, and the function
 * returns NULL.
 *
 * @param prefix Prefix for the temporary file name (e.g., "test").
 * @param temp_path Buffer to store the path to the temporary file.
 * @param path_size Size of the temp_path buffer.
 * @return char*
 *         - Pointer to `temp_path` containing the path to the temporary file if successful.
 *         - NULL if an error occurred (e.g., invalid parameters, failed to create file).
 * @note This function is currently unused in the system.
 */
char* create_temp_file(const char *prefix, char *temp_path, size_t path_size);

/**
 * @brief Read a specific portion of a file starting from an offset
 *
 * This function reads a portion of the specified file starting from the given offset
 * into a buffer. The number of bytes read is stored in the `bytes_read` parameter.
 * If the file cannot be opened, the offset is invalid, or a read error occurs, an error
 * is logged, and the function returns -1.
 *
 * @param file_path Path to the file to read (e.g., "config/ping.json").
 * @param buffer Buffer to store the read data.
 * @param buffer_size Size of the buffer.
 * @param offset Position in the file to start reading from.
 * @param bytes_read Pointer to a size_t variable where the number of bytes read will be stored.
 * @return int
 *         - 0 if the read operation was successful.
 *         - -1 if an error occurred (e.g., file not found, read error).
 * @note This function is currently unused in the system.
 */
int read_file_chunk(const char *file_path, char *buffer, size_t buffer_size, off_t offset, size_t *bytes_read);

#endif /* FILE_PROCESS_H */