#ifndef FILE_PROCESS_H
#define FILE_PROCESS_H

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

/**
 * @brief Read the entire contents of a file into a buffer
 * 
 * @param file_path Path to the file to be read
 * @param buffer Pointer to the buffer pointer that will be allocated
 * @param size Pointer to variable that will store the size of data read
 * @return int 0 if successful, -1 if failed
 */
int read_file(const char *file_path, char **buffer, size_t *size);

/**
 * @brief Write data to a file
 * 
 * @param file_path Path to the file to be written
 * @param buffer Pointer to the buffer containing data
 * @param size Size of data to be written
 * @return int 0 if successful, -1 if failed
 */
int write_file(const char *file_path, const char *buffer, size_t size);

/**
 * @brief Append data to the end of a file
 * 
 * @param file_path Path to the file
 * @param buffer Pointer to the buffer containing data
 * @param size Size of data to be appended
 * @return int 0 if successful, -1 if failed
 */
int append_to_file(const char *file_path, const char *buffer, size_t size);

/**
 * @brief Check if a file exists
 * 
 * @param file_path Path to the file
 * @return int 1 if exists, 0 if not exists
 */
int file_exists(const char *file_path);

/**
 * @brief Create a new directory
 * 
 * @param dir_path Path to the directory to be created
 * @return int 0 if successful, -1 if failed
 */
int create_directory(const char *dir_path);

/**
 * @brief Delete a file
 * 
 * @param file_path Path to the file to be deleted
 * @return int 0 if successful, -1 if failed
 */
int delete_file(const char *file_path);

/**
 * @brief Copy a file
 * 
 * @param src_path Path to the source file
 * @param dest_path Path to the destination file
 * @return int 0 if successful, -1 if failed
 */
int copy_file(const char *src_path, const char *dest_path);

/**
 * @brief Get the size of a file
 * 
 * @param file_path Path to the file
 * @return long Size of the file, -1 if failed
 */
long get_file_size(const char *file_path);

/**
 * @brief Get the last modification time of a file
 * 
 * @param file_path Path to the file
 * @return time_t Modification time, -1 if failed
 */
time_t get_file_modification_time(const char *file_path);

/**
 * @brief Create a temporary file
 * 
 * @param prefix Prefix for the filename
 * @param temp_path Buffer to store the path to the temporary file
 * @param path_size Size of the temp_path buffer
 * @return char* Pointer to temp_path if successful, NULL if failed
 */
char* create_temp_file(const char *prefix, char *temp_path, size_t path_size);

/**
 * @brief Read a portion of a file from a specific offset
 * 
 * @param file_path Path to the file
 * @param buffer Buffer to store the read data
 * @param buffer_size Size of the buffer
 * @param offset Position to start reading from
 * @param bytes_read Pointer to variable that will store the number of bytes read
 * @return int 0 if successful, -1 if failed
 */
int read_file_chunk(const char *file_path, char *buffer, size_t buffer_size, off_t offset, size_t *bytes_read);

#endif /* FILE_PROCESS_H */