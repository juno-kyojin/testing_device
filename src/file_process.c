/**
 * @file file_process.c
 * @brief Implementation of file operations for Linux systems
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdarg.h>
 #include <time.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <zlib.h>
 
 #include "../include/file_process.h"
 
 #define COMPRESS_CHUNK_SIZE 16384
 
 FILE* open_file(const char* filename, file_mode_t mode) {
     if (!filename) {
         return NULL;
     }
 
     const char* mode_str = NULL;
     switch (mode) {
         case FILE_MODE_READ:
             mode_str = "r";
             break;
         case FILE_MODE_WRITE:
             mode_str = "w";
             break;
         case FILE_MODE_APPEND:
             mode_str = "a";
             break;
         case FILE_MODE_READ_WRITE:
             mode_str = "r+";
             break;
         default:
             return NULL;
     }
 
     // Make sure directory exists for write and append modes
     if (mode == FILE_MODE_WRITE || mode == FILE_MODE_APPEND) {
         char* dir_path = strdup(filename);
         if (!dir_path) {
             return NULL;
         }
         
         // Find the last directory separator
         char* last_sep = strrchr(dir_path, '/');
         if (last_sep) {
             *last_sep = '\0';  // Null-terminate at the last separator
             
             // Create directory path if it doesn't exist
             struct stat st = {0};
             if (strlen(dir_path) > 0 && stat(dir_path, &st) == -1) {
                 char cmd[512];
                 snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir_path);
                 system(cmd);
             }
         }
         free(dir_path);
     }
 
     FILE* fp = fopen(filename, mode_str);
     return fp;
 }
 
 ssize_t read_file(const char* filename, void* buffer, size_t buffer_size) {
     if (!filename || !buffer || buffer_size == 0) {
         return -1;
     }
 
     FILE* fp = open_file(filename, FILE_MODE_READ);
     if (!fp) {
         return -1;
     }
 
     // Get file size
     fseek(fp, 0, SEEK_END);
     long file_size = ftell(fp);
     rewind(fp);
 
     if (file_size <= 0) {
         fclose(fp);
         return 0;
     }
 
     if ((size_t)file_size > buffer_size) {
         fclose(fp);
         return -1;  // Buffer too small
     }
 
     size_t bytes_read = fread(buffer, 1, file_size, fp);
     fclose(fp);
 
     if (bytes_read != (size_t)file_size) {
         return -1;
     }
 
     return bytes_read;
 }
 
 ssize_t read_batch_file(FILE* fp, void* buffer, size_t buffer_size) {
     if (!fp || !buffer || buffer_size == 0) {
         return -1;
     }
 
     size_t bytes_read = fread(buffer, 1, buffer_size, fp);
     if (bytes_read == 0 && ferror(fp)) {
         return -1;
     }
 
     return bytes_read;
 }
 
 ssize_t write_to_file(const char* filename, const void* data, size_t data_size, bool append) {
     if (!filename || !data || data_size == 0) {
         return -1;
     }
 
     FILE* fp = open_file(filename, append ? FILE_MODE_APPEND : FILE_MODE_WRITE);
     if (!fp) {
         return -1;
     }
 
     size_t bytes_written = fwrite(data, 1, data_size, fp);
     fclose(fp);
 
     if (bytes_written != data_size) {
         return -1;
     }
 
     return bytes_written;
 }
 
 ssize_t write_batch_file(FILE* fp, const void* data, size_t data_size) {
     if (!fp || !data || data_size == 0) {
         return -1;
     }
 
     size_t bytes_written = fwrite(data, 1, data_size, fp);
     if (bytes_written != data_size) {
         return -1;
     }
 
     // Flush to ensure data is written to disk
     fflush(fp);
 
     return bytes_written;
 }
 
 int printf_time_to_file(FILE* fp, const char* format, ...) {
     if (!fp || !format) {
         return -1;
     }
 
     // Get current time
     time_t now = time(NULL);
     struct tm* timeinfo = localtime(&now);
     
     // Write timestamp
     char timestamp[64];
     strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", timeinfo);
     fputs(timestamp, fp);
     
     // Write formatted message
     va_list args;
     va_start(args, format);
     int ret = vfprintf(fp, format, args);
     va_end(args);
     
     // Always add a newline at the end
     if (format[strlen(format) - 1] != '\n') {
         fputc('\n', fp);
     }
     
     fflush(fp);
     
     return ret;
 }
 
 int printf_time_to_file_custom(FILE* fp, const char* time_format, const char* msg_format, ...) {
     if (!fp || !time_format || !msg_format) {
         return -1;
     }
 
     // Get current time
     time_t now = time(NULL);
     struct tm* timeinfo = localtime(&now);
     
     // Write custom timestamp
     char timestamp[128];
     strftime(timestamp, sizeof(timestamp), time_format, timeinfo);
     fputs(timestamp, fp);
     
     // Write formatted message
     va_list args;
     va_start(args, msg_format);
     int ret = vfprintf(fp, msg_format, args);
     va_end(args);
     
     fflush(fp);
     
     return ret;
 }
 
 int compress_file(const char* src_filename, const char* dst_filename, compress_level_t level) {
     if (!src_filename || !dst_filename) {
         return -1;
     }
 
     // Map compression level to zlib levels
     int zlib_level;
     switch (level) {
         case COMPRESS_LEVEL_FAST:
             zlib_level = 1;  // Z_BEST_SPEED
             break;
         case COMPRESS_LEVEL_DEFAULT:
             zlib_level = 6;  // Z_DEFAULT_COMPRESSION
             break;
         case COMPRESS_LEVEL_BEST:
             zlib_level = 9;  // Z_BEST_COMPRESSION
             break;
         default:
             zlib_level = 6;  // Default compression
     }
 
     // Open input file
     FILE* source = fopen(src_filename, "rb");
     if (!source) {
         return -1;
     }
 
     // Open output file
     gzFile dest = gzopen(dst_filename, "wb");
     if (!dest) {
         fclose(source);
         return -1;
     }
 
     // Set compression level
     gzsetparams(dest, zlib_level, Z_DEFAULT_STRATEGY);
 
     // Compress data in chunks
     unsigned char in[COMPRESS_CHUNK_SIZE];
     int bytes_read;
     int total_bytes = 0;
 
     while ((bytes_read = fread(in, 1, sizeof(in), source)) > 0) {
         if (gzwrite(dest, in, bytes_read) != bytes_read) {
             fclose(source);
             gzclose(dest);
             return -1;
         }
         total_bytes += bytes_read;
     }
 
     fclose(source);
     gzclose(dest);
 
     return 0;
 }
 
 int decompress_file(const char* src_filename, const char* dst_filename) {
     if (!src_filename || !dst_filename) {
         return -1;
     }
 
     // Open compressed input file
     gzFile source = gzopen(src_filename, "rb");
     if (!source) {
         return -1;
     }
 
     // Open output file
     FILE* dest = fopen(dst_filename, "wb");
     if (!dest) {
         gzclose(source);
         return -1;
     }
 
     // Decompress data in chunks
     unsigned char out[COMPRESS_CHUNK_SIZE];
     int bytes_read;
     int total_bytes = 0;
 
     while ((bytes_read = gzread(source, out, sizeof(out))) > 0) {
         if (fwrite(out, 1, bytes_read, dest) != (size_t)bytes_read) {
             gzclose(source);
             fclose(dest);
             return -1;
         }
         total_bytes += bytes_read;
     }
 
     gzclose(source);
     fclose(dest);
 
     return 0;
 }
 
 int clear_file_to_run(const char* filename) {
     if (!filename) {
         return -1;
     }
 
     FILE* fp = open_file(filename, FILE_MODE_WRITE);
     if (!fp) {
         return -1;
     }
 
     // Close the file immediately - opening in "w" mode truncates it
     fclose(fp);
 
     return 0;
 }