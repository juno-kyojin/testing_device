# README - Hệ thống `testing_device`

## Mục lục

- [Giới thiệu](#giới-thiệu)
- [Cách hoạt động của hệ thống](#cách-hoạt-động-của-hệ-thống)
  - [Quy trình tổng quan](#quy-trình-tổng-quan)
  - [Cấu trúc file test case](#cấu-trúc-file-test-case)
- [Hướng dẫn thêm test case mới](#hướng-dẫn-thêm-test-case-mới)
  - [Tạo file test case JSON](#tạo-file-test-case-json)
  - [Viết handler cho service mới](#viết-handler-cho-service-mới)
  - [Đăng ký service vào hệ thống](#đăng-ký-service-vào-hệ-thống)
  - [Cập nhật Makefile](#cập-nhật-makefile)
  - [Build và kiểm tra](#build-và-kiểm-tra)
- [Lưu ý và mẹo sử dụng](#lưu-ý-và-mẹo-sử-dụng)
- [Kết luận](#kết-luận)

---

## Giới thiệu

Hệ thống `testing_device` là một ứng dụng tự động thực thi các test case được định nghĩa trong các file JSON. Hệ thống được thiết kế để:

- Giám sát thư mục `config` để phát hiện các file test case mới (file JSON).
- Parse file JSON để lấy danh sách test case.
- Thực thi các test case theo service được chỉ định.
- Ghi kết quả vào thư mục `result` và di chuyển file đã xử lý vào thư mục `processed`.

Hệ thống chạy như một dịch vụ systemd (`testing_device.service`), tự động khởi động khi hệ thống khởi động, đảm bảo hoạt động liên tục.

---

## Cách hoạt động của hệ thống

### Quy trình tổng quan

Hệ thống `testing_device` hoạt động theo các bước sau:

1. **Giám sát thư mục `config`**:
   - Sử dụng `inotify` để phát hiện các file JSON mới trong thư mục `config`.
   - Khi phát hiện file JSON (như `ping.json`), hệ thống thêm đường dẫn file vào một hàng đợi (queue) để xử lý tuần tự.

2. **Lấy file từ hàng đợi**:
   - Hệ thống lấy file từ hàng đợi theo thứ tự phát hiện (first-come, first-served).
   - Gọi hàm xử lý để đọc và thực thi file JSON.

3. **Đọc và parse file JSON**:
   - Đọc toàn bộ nội dung file JSON vào bộ nhớ.
   - Parse nội dung JSON để lấy danh sách test case (mảng `test_cases`).
   - Mỗi test case chứa thông tin: `service` (bắt buộc), `action` (tùy chọn), và `params` (tùy chọn).

4. **Thực thi test case**:
   - Với mỗi test case, hệ thống tra cứu handler (hàm xử lý) tương ứng với `service`.
   - Gọi handler để thực thi test case (ví dụ: ping host, kiểm tra tốc độ mạng).
   - Ghi kết quả thực thi vào một mảng JSON (`result_array`).

5. **Ghi kết quả và di chuyển file**:
   - Sau khi thực thi tất cả test case, kết quả được ghi vào file JSON trong thư mục `result` (ví dụ: `result/ping.json_20250506120000_result.json`).
   - File JSON gốc được di chuyển vào thư mục `processed` (ví dụ: `processed/ping.json`) để tránh xử lý lại.

### Cấu trúc file test case

File test case là một file JSON chứa danh sách các test case. Cấu trúc cơ bản như sau:

- **`"test_cases"`**: Một mảng chứa danh sách các test case.
- **Mỗi test case**:
  - `service` (bắt buộc): Tên dịch vụ (ví dụ: `"ping"`).
  - `action` (tùy chọn): Hành động cụ thể (mặc định là `"default"` nếu không khai báo).
  - `params` (tùy chọn): Tham số cho test case, dạng JSON object (ví dụ: `{"host": "youtube.com"}`).

**Ví dụ file `ping.json`**:

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

## Hướng dẫn thêm test case mới

Nếu bạn muốn thêm một test case mới (ví dụ: kiểm tra trạng thái website với service `http_check`), hãy làm theo các bước sau:

### Tạo file test case JSON

1. **Tạo file JSON**:
   - Tạo một file JSON mới trong thư mục `config`, ví dụ: `config/http_check.json`.
   - Nội dung file có cấu trúc như sau:

     ```json
     {
         "test_cases": [
             {
                 "service": "http_check",
                 "params": {
                     "url": "https://example.com"
                 }
             },
             {
                 "service": "http_check",
                 "params": {
                     "url": "https://google.com"
                 }
             }
         ]
     }
     ```

2. **Đặt file vào thư mục `config`**:
   - Copy file vào thư mục `config`:
     ```
     cp http_check.json /path/to/testing_device/config/
     ```
   - Hệ thống sẽ tự động phát hiện file và xử lý.

### Viết handler cho service mới

Nếu service `http_check` chưa được hỗ trợ, bạn cần viết một handler cho nó:

1. **Tạo file header**:
   - Tạo file `include/http_check.h`.
   - Khai báo prototype của handler:

     ```c
     void executeHttpCheck(TestCase *test_case, const char *filepath, int index, cJSON *result_array);
     ```

2. **Tạo file source**:
   - Tạo file `src/test/http_check.c`.
   - Triển khai hàm `executeHttpCheck`:
     - Parse tham số `url` từ `params`.
     - Gửi HTTP request để kiểm tra trạng thái website (có thể dùng thư viện như `libcurl`).
     - Ghi kết quả vào `result_array` (pass nếu mã trạng thái là 200, fail nếu không).

### Đăng ký service vào hệ thống

1. **Mở file `action_registry.c`**:
   - File này chứa hàm `init_action_dispatch`, nơi các service được đăng ký.

2. **Thêm lệnh đăng ký**:
   - Trong hàm `init_action_dispatch`, thêm dòng sau:

     ```c
     register_service("http_check", executeHttpCheck);
     ```

   - Dòng này ánh xạ `service: "http_check"` với handler `executeHttpCheck`.

### Cập nhật Makefile

1. **Mở file `Makefile`**:
   - Thêm file mới vào danh sách `SOURCES` và `OBJECTS`.

2. **Cập nhật danh sách**:
   - Thêm `$(TEST_DIR)/http_check.c` vào `SOURCES`.
   - Thêm `$(BIN_DIR)/http_check.o` vào `OBJECTS`.
   - Thêm `$(INCLUDE_DIR)/http_check.h` vào `HEADERS`.
   - Ví dụ:

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
         $(TEST_DIR)/http_check.c

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
         $(BIN_DIR)/http_check.o

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
         $(INCLUDE_DIR)/http_check.h
     ```

### Build và kiểm tra

1. **Build lại hệ thống**:
   - Chạy lệnh để làm sạch và build lại:

     ```
     make clean
     make
     ```

2. **Kiểm tra kết quả**:
   - Đảm bảo dịch vụ `testing_device.service` đang chạy:

     ```
     sudo systemctl status testing_device.service
     ```

     - Nếu chưa chạy, khởi động dịch vụ:

       ```
       sudo systemctl start testing_device.service
       ```

   - Đặt file `http_check.json` vào thư mục `config`.
   - Kiểm tra file kết quả trong thư mục `result` (ví dụ: `result/http_check.json_20250506120000_result.json`).
   - Kiểm tra log để xem quá trình thực thi (thường trong `/var/log/testing_device.log` hoặc stdout/stderr).

---

## Lưu ý và mẹo sử dụng

- **Đảm bảo tên file JSON duy nhất**:
  - Khi đặt file JSON vào `config`, hãy đảm bảo tên file là duy nhất (ví dụ: thêm timestamp như `ping_20250506120000.json`) để tránh ghi đè trong thư mục `processed`.

- **Kiểm tra log**:
  - Log được ghi bởi hệ thống rất hữu ích để gỡ lỗi. Xem log để biết file có được phát hiện, xử lý, và ghi kết quả đúng không.

- **Thời gian xử lý**:
  - Hệ thống xử lý file JSON tuần tự (theo hàng đợi). Nếu một file chứa nhiều test case hoặc test case mất nhiều thời gian (như ping host), các file sau sẽ phải chờ. Hãy cân nhắc số lượng test case trong mỗi file.

- **Kiểm tra service hỗ trợ**:
  - Trước khi thêm test case mới, kiểm tra xem `service` đã được hỗ trợ chưa (xem trong `init_action_dispatch` của `action_registry.c`). Nếu chưa, bạn cần viết handler mới.

- **Sao lưu file trước khi chạy**:
  - File JSON sẽ được di chuyển từ `config` sang `processed` sau khi xử lý. Hãy sao lưu file nếu bạn cần dùng lại.

---

## Kết luận

Hệ thống `testing_device` cung cấp một cách linh hoạt để thực thi các test case tự động. Người dùng có thể dễ dàng thêm test case mới bằng cách tạo file JSON và viết handler nếu cần. Nếu bạn gặp vấn đề hoặc cần hỗ trợ thêm, hãy kiểm tra log và liên hệ với nhóm phát triển.