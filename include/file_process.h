/**
 * @file file_process.h
 * @brief File operations management for Linux systems
 *
 * This module provides functions for reading, writing, compressing,
 * and decompressing files on Linux file systems, optimized for
 * mass batch processing.
 */

 #ifndef FILE_PROCESS_H
 #define FILE_PROCESS_H
 
 #include <stdio.h>
 #include <stdint.h>
 #include <stdbool.h>
 #include <sys/types.h>
 
 /**
  * @brief File operation modes
  */
 typedef enum {
     FILE_MODE_READ,       /**< Open file for reading */
     FILE_MODE_WRITE,      /**< Open file for writing */
     FILE_MODE_APPEND,     /**< Open file for appending */
     FILE_MODE_READ_WRITE  /**< Open file for both reading and writing */
 } file_mode_t;
 
 /**
  * @brief Compression levels for file compression
  */
 typedef enum {
     COMPRESS_LEVEL_FAST,   /**< Fast compression with lower ratio */
     COMPRESS_LEVEL_DEFAULT, /**< Default compression level */
     COMPRESS_LEVEL_BEST     /**< Best compression with slower speed */
 } compress_level_t;
 
 /**
  * @brief Opens a file with specified mode
  *
  * @param filename Path to the file to open
  * @param mode File operation mode
  * @return FILE* File pointer if successful, NULL otherwise
  */
 FILE* open_file(const char* filename, file_mode_t mode);
 
 /**
  * @brief Reads entire file content into a buffer
  *
  * @param filename Path to the file to read
  * @param buffer Pointer to buffer to store content
  * @param buffer_size Size of the buffer
  * @return ssize_t Number of bytes read or -1 on error
  */
 ssize_t read_file(const char* filename, void* buffer, size_t buffer_size);
 
 /**
  * @brief Reads a batch of data from file
  *
  * @param fp File pointer to read from
  * @param buffer Pointer to buffer to store content
  * @param buffer_size Size of the buffer
  * @return ssize_t Number of bytes read or -1 on error
  */
 ssize_t read_batch_file(FILE* fp, void* buffer, size_t buffer_size);
 
 /**
  * @brief Writes data to a file
  *
  * @param filename Path to the file to write
  * @param data Data to write
  * @param data_size Size of data in bytes
  * @param append Whether to append to file or overwrite
  * @return ssize_t Number of bytes written or -1 on error
  */
 ssize_t write_to_file(const char* filename, const void* data, size_t data_size, bool append);
 
 /**
  * @brief Writes a batch of data to file
  *
  * @param fp File pointer to write to
  * @param data Data to write
  * @param data_size Size of data in bytes
  * @return ssize_t Number of bytes written or -1 on error
  */
 ssize_t write_batch_file(FILE* fp, const void* data, size_t data_size);
 
 /**
  * @brief Writes formatted timestamp and message to a file
  *
  * @param fp File pointer to write to
  * @param format Format string for message
  * @return int Number of characters written or negative value on error
  */
 int printf_time_to_file(FILE* fp, const char* format, ...);
 
 /**
  * @brief Writes custom formatted timestamp and message to a file
  *
  * @param fp File pointer to write to
  * @param time_format Format string for timestamp
  * @param msg_format Format string for message
  * @return int Number of characters written or negative value on error
  */
 int printf_time_to_file_custom(FILE* fp, const char* time_format, const char* msg_format, ...);
 
 /**
  * @brief Compresses a file using zlib
  *
  * @param src_filename Source file to compress
  * @param dst_filename Destination compressed file
  * @param level Compression level
  * @return int 0 on success, negative value on error
  */
 int compress_file(const char* src_filename, const char* dst_filename, compress_level_t level);
 
 /**
  * @brief Decompresses a file using zlib
  *
  * @param src_filename Source compressed file
  * @param dst_filename Destination decompressed file
  * @return int 0 on success, negative value on error
  */
 int decompress_file(const char* src_filename, const char* dst_filename);
 
 /**
  * @brief Clears all content from a file
  *
  * @param filename File to clear
  * @return int 0 on success, negative value on error
  */
 int clear_file_to_run(const char* filename);
 
 #endif /* FILE_PROCESS_H */