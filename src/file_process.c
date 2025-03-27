

 #include "file_process.h"
 #include "log.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <unistd.h>
 #include <fcntl.h>
 
 int read_file(const char *file_path, char **buffer, size_t *size) {
     if (!file_path || !buffer || !size) {
         log_message(LOG_LVL_ERROR, "read_file: Invalid parameters");
         return -1;
     }
     
     *buffer = NULL;
     *size = 0;
     
     FILE *file = fopen(file_path, "rb");
     if (!file) {
         log_message(LOG_LVL_ERROR, "Failed to open file %s: %s", file_path, strerror(errno));
         return -1;
     }
     
     // Lấy kích thước file
     fseek(file, 0, SEEK_END);
     long file_size = ftell(file);
     if (file_size < 0) {
         log_message(LOG_LVL_ERROR, "Failed to determine size of file %s: %s", file_path, strerror(errno));
         fclose(file);
         return -1;
     }
     fseek(file, 0, SEEK_SET);
     
     // Cấp phát bộ nhớ
     *buffer = (char *)malloc(file_size + 1);
     if (!*buffer) {
         log_message(LOG_LVL_ERROR, "Memory allocation failed for file %s", file_path);
         fclose(file);
         return -1;
     }
     
     // Đọc nội dung file
     size_t bytes_read = fread(*buffer, 1, file_size, file);
     if (bytes_read != file_size) {
         log_message(LOG_LVL_ERROR, "Failed to read entire file %s: %s", file_path, strerror(errno));
         free(*buffer);
         *buffer = NULL;
         fclose(file);
         return -1;
     }
     
     // Thêm null terminator cho chuỗi
     (*buffer)[file_size] = '\0';
     *size = file_size;
     
     fclose(file);
     log_message(LOG_LVL_DEBUG, "Successfully read %lu bytes from file %s", (unsigned long)file_size, file_path);
     return 0;
 }
 
 int write_file(const char *file_path, const char *buffer, size_t size) {
     if (!file_path || (!buffer && size > 0)) {
         log_message(LOG_LVL_ERROR, "write_file: Invalid parameters");
         return -1;
     }
     
     FILE *file = fopen(file_path, "wb");
     if (!file) {
         log_message(LOG_LVL_ERROR, "Failed to open file for writing %s: %s", file_path, strerror(errno));
         return -1;
     }
     
     // Ghi nội dung vào file
     size_t bytes_written = fwrite(buffer, 1, size, file);
     if (bytes_written != size) {
         log_message(LOG_LVL_ERROR, "Failed to write entire content to file %s: %s", file_path, strerror(errno));
         fclose(file);
         return -1;
     }
     
     fclose(file);
     log_message(LOG_LVL_DEBUG, "Successfully wrote %lu bytes to file %s", (unsigned long)size, file_path);
     return 0;
 }
 
 int append_to_file(const char *file_path, const char *buffer, size_t size) {
     if (!file_path || (!buffer && size > 0)) {
         log_message(LOG_LVL_ERROR, "append_to_file: Invalid parameters");
         return -1;
     }
     
     FILE *file = fopen(file_path, "ab");
     if (!file) {
         log_message(LOG_LVL_ERROR, "Failed to open file for appending %s: %s", file_path, strerror(errno));
         return -1;
     }
     
     // Ghi thêm nội dung vào file
     size_t bytes_written = fwrite(buffer, 1, size, file);
     if (bytes_written != size) {
         log_message(LOG_LVL_ERROR, "Failed to append entire content to file %s: %s", file_path, strerror(errno));
         fclose(file);
         return -1;
     }
     
     fclose(file);
     log_message(LOG_LVL_DEBUG, "Successfully appended %lu bytes to file %s", (unsigned long)size, file_path);
     return 0;
 }
 
 int file_exists(const char *file_path) {
     if (!file_path) {
         return 0;
     }
     return access(file_path, F_OK) == 0;
 }
 
 int create_directory(const char *dir_path) {
     if (!dir_path) {
         log_message(LOG_LVL_ERROR, "create_directory: Invalid parameter");
         return -1;
     }
     
     // Tạo thư mục với quyền 0755 (rwxr-xr-x)
     if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
         log_message(LOG_LVL_ERROR, "Failed to create directory %s: %s", dir_path, strerror(errno));
         return -1;
     } else if (errno == EEXIST) {
         log_message(LOG_LVL_DEBUG, "Directory %s already exists", dir_path);
     } else {
         log_message(LOG_LVL_DEBUG, "Successfully created directory %s", dir_path);
     }
     
     return 0;
 }
 
 int delete_file(const char *file_path) {
     if (!file_path) {
         log_message(LOG_LVL_ERROR, "delete_file: Invalid parameter");
         return -1;
     }
     
     if (unlink(file_path) != 0) {
         log_message(LOG_LVL_ERROR, "Failed to delete file %s: %s", file_path, strerror(errno));
         return -1;
     }
     
     log_message(LOG_LVL_DEBUG, "Successfully deleted file %s", file_path);
     return 0;
 }
 
 int copy_file(const char *src_path, const char *dest_path) {
     char buffer[4096];
     size_t bytes_read;
     
     if (!src_path || !dest_path) {
         log_message(LOG_LVL_ERROR, "copy_file: Invalid parameters");
         return -1;
     }
     
     FILE *src_file = fopen(src_path, "rb");
     if (!src_file) {
         log_message(LOG_LVL_ERROR, "Failed to open source file %s: %s", src_path, strerror(errno));
         return -1;
     }
     
     FILE *dest_file = fopen(dest_path, "wb");
     if (!dest_file) {
         log_message(LOG_LVL_ERROR, "Failed to open destination file %s: %s", dest_path, strerror(errno));
         fclose(src_file);
         return -1;
     }
     
     // Copy nội dung
     while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
         if (fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
             log_message(LOG_LVL_ERROR, "Error writing to destination file %s: %s", dest_path, strerror(errno));
             fclose(src_file);
             fclose(dest_file);
             return -1;
         }
     }
     
     if (ferror(src_file)) {
         log_message(LOG_LVL_ERROR, "Error reading from source file %s: %s", src_path, strerror(errno));
         fclose(src_file);
         fclose(dest_file);
         return -1;
     }
     
     fclose(src_file);
     fclose(dest_file);
     
     log_message(LOG_LVL_DEBUG, "Successfully copied file %s to %s", src_path, dest_path);
     return 0;
 }
 
 long get_file_size(const char *file_path) {
     if (!file_path) {
         return -1;
     }
     
     struct stat st;
     if (stat(file_path, &st) != 0) {
         log_message(LOG_LVL_ERROR, "Failed to get file size for %s: %s", file_path, strerror(errno));
         return -1;
     }
     
     return st.st_size;
 }
 
 time_t get_file_modification_time(const char *file_path) {
     if (!file_path) {
         return -1;
     }
     
     struct stat st;
     if (stat(file_path, &st) != 0) {
         log_message(LOG_LVL_ERROR, "Failed to get modification time for %s: %s", file_path, strerror(errno));
         return -1;
     }
     
     return st.st_mtime;
 }
 
 char* create_temp_file(const char *prefix, char *temp_path, size_t path_size) {
     if (!prefix || !temp_path || path_size < 16) {
         log_message(LOG_LVL_ERROR, "create_temp_file: Invalid parameters");
         return NULL;
     }
     
     snprintf(temp_path, path_size, "/tmp/%s-XXXXXX", prefix);
     
     int fd = mkstemp(temp_path);
     if (fd == -1) {
         log_message(LOG_LVL_ERROR, "Failed to create temp file with prefix %s: %s", prefix, strerror(errno));
         return NULL;
     }
     
     close(fd);
     log_message(LOG_LVL_DEBUG, "Successfully created temp file %s", temp_path);
     return temp_path;
 }
 
 int read_file_chunk(const char *file_path, char *buffer, size_t buffer_size, off_t offset, size_t *bytes_read) {
     if (!file_path || !buffer || !bytes_read) {
         log_message(LOG_LVL_ERROR, "read_file_chunk: Invalid parameters");
         return -1;
     }
     
     *bytes_read = 0;
     
     int fd = open(file_path, O_RDONLY);
     if (fd == -1) {
         log_message(LOG_LVL_ERROR, "Failed to open file %s: %s", file_path, strerror(errno));
         return -1;
     }
     
     // Di chuyển con trỏ đến vị trí offset
     if (lseek(fd, offset, SEEK_SET) == -1) {
         log_message(LOG_LVL_ERROR, "Failed to seek to offset %ld in file %s: %s", (long)offset, file_path, strerror(errno));
         close(fd);
         return -1;
     }
     
     // Đọc dữ liệu
     ssize_t read_size = read(fd, buffer, buffer_size);
     if (read_size == -1) {
         log_message(LOG_LVL_ERROR, "Failed to read from file %s: %s", file_path, strerror(errno));
         close(fd);
         return -1;
     }
     
     *bytes_read = (size_t)read_size;
     close(fd);
     
     log_message(LOG_LVL_DEBUG, "Successfully read %lu bytes from file %s at offset %ld", (unsigned long)read_size, file_path, (long)offset);
     return 0;
 }