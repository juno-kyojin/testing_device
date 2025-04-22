# Hệ Thống Kiểm Thử Thiết Bị

## Tổng Quan

**Hệ Thống Kiểm Thử Thiết Bị** là một công cụ được thiết kế để thực hiện các bài kiểm tra chẩn đoán trên thiết bị mạng. 

Tài liệu này cung cấp hướng dẫn cơ bản để xây dựng, chạy hệ thống và đọc log kết quả.

## Yêu Cầu Cần Có

Trước khi sử dụng hệ thống, hãy đảm bảo bạn đã cài đặt các công cụ sau trên hệ điều hành Linux:

- **Trình Biên Dịch GCC**: Để xây dựng dự án.

  ```
  sudo apt-get install gcc
  ```
- **Make**: Để xây dựng dự án bằng Makefile.

  ```
  sudo apt-get install make
  ```
- **Thư Viện cJSON**: Để xử lý các file cấu hình JSON.

  ```
  sudo apt-get install libcjson-dev
  ```
- **speedtest-cli**: Cần thiết cho bài kiểm tra `speedtest` để đo tốc độ mạng (tùy chọn nếu không dùng `speedtest`).

  ```
  sudo apt-get install speedtest-cli
  ```

## Cấu Trúc Thư Mục

- `config/`: Chứa các file cấu hình bài kiểm tra định dạng JSON (`ping.json`, `speedtest.json`, v.v.).
- `src/`: Chứa các file mã nguồn.
- `include/`: Chứa các file header.
- `build/`: Thư mục chứa file thực thi sau khi biên dịch (`testing_device`).
- `bin/`: Thư mục tạm để lưu các file đối tượng trong quá trình biên dịch.

## Biên Dịch Dự Án

1. Di chuyển đến thư mục dự án:

   ```
   cd testing_device
   ```

2. Biên dịch dự án bằng Makefile:

   ```
   make
   ```

   - Lệnh này sẽ biên dịch mã nguồn và tạo file thực thi `build/testing_device`.

3. Nếu cần xóa các file biên dịch:

   ```
   make clean
   ```

## Chạy Các Bài Kiểm Tra

### Chạy Các Bài Kiểm Tra Cụ Thể

Bạn có thể chạy các bài kiểm tra cụ thể bằng cách cung cấp đường dẫn đến các file cấu hình JSON qua tham số dòng lệnh.

**Ví dụ**:

```
./build/testing_device config/ping.json config/speedtest.json
```

- Lệnh này chạy bài kiểm tra `ping` (từ `ping.json`) và sau đó là bài kiểm tra `speedtest` (từ `speedtest.json`).



## Các Bài Kiểm Tra Hiện Có

### 1. `ping`

- **Mục đích**: Kiểm tra khả năng kết nối mạng bằng cách gửi các gói tin ICMP đến một host được chỉ định.
- **File Cấu Hình**: `config/ping.json`
- **Tham Số Đầu Vào**:
  - `host`: Host mục tiêu để ping (ví dụ: `"google.com"`).
  - `plugin`: Tên plugin tùy chọn (ví dụ: `"Debug Ping Plugin"`).
- **Kết Quả Đầu Ra**:
  - `Ping.Status`: 0 (thành công) hoặc 1 (thất bại).
  - `Ping.host`: Host mục tiêu.
  - `Ping.hostAddress`: Địa chỉ IP của host mục tiêu.
  - `Ping.successCount`: Số gói tin nhận thành công.
  - `Ping.failureCount`: Số gói tin bị mất.
  - `Ping.averageResponseTime`: Thời gian phản hồi trung bình (ms).
  - `Ping.minimumResponseTime`: Thời gian phản hồi nhỏ nhất (ms).
  - `Ping.maximumResponseTime`: Thời gian phản hồi lớn nhất (ms).
  - `Ping.jitter`: Độ dao động mạng (ms).
  - `Ping.packetLoss`: Tỷ lệ mất gói (%).

### 2. `speedtest`

- **Mục đích**: Đo tốc độ mạng (tốc độ tải xuống, tải lên và độ trễ).
- **File Cấu Hình**: `config/speedtest.json`
- **Tham Số Đầu Vào** (tùy chọn):
  - `server`: Tên server speedtest (ví dụ: `"speedtest.net"`). Nếu không chỉ định, hệ thống sẽ dùng server gần nhất.
- **Kết Quả Đầu Ra**:
  - `Speedtest.Status`: 0 (thành công) hoặc 1 (thất bại).
  - `Server`: Tên server được sử dụng để kiểm tra.
  - `Server Location`: Thành phố và quốc gia của server.
  - `ISP`: Nhà cung cấp dịch vụ internet được sử dụng.
  - `Download`: Tốc độ tải xuống (Mbps).
  - `Upload`: Tốc độ tải lên (Mbps).
  - `Latency`: Độ trễ mạng (ms).

## Đọc Log Kết Quả

Hệ thống tạo log chi tiết để giúp bạn hiểu quá trình thực thi và kết quả bài kiểm tra. Log được ghi vào file `application.log` (nếu được cấu hình trong `config.json`) và cũng hiển thị trên màn hình console.

### Cấu Trúc Log

- **Thời Gian**: Mỗi dòng log bắt đầu bằng thời gian (ví dụ: `[2025-04-22 11:00:54]`).
- **Mức Độ Log**: Thể hiện mức độ quan trọng (`DEBUG`, `INFO`, `WARN`, `ERROR`).
- **Thông Điệp**: Mô tả hành động hoặc kết quả.

### Các Phần Chính Cần Chú Ý

1. **Tải Bài Kiểm Tra**:

   - Dòng như `Processing test configuration file: config/ping.json` cho biết hệ thống đang tải một bài kiểm tra.
   - `Successfully read XXX bytes from file` và `Successfully parsed X test cases` xác nhận file đã được tải và phân tích thành công.

2. **Thực Thi Bài Kiểm Tra**:

   - `Executing action: ping (type: diagnostic)`: Hiển thị hành động đang được thực thi và loại hành động.
   - `Executing ping test`: Cho biết trình xử lý (`execute_ping`) đang chạy.
   - `Param host = ...`: Hiển thị các tham số đầu vào của bài kiểm tra.

3. **Kết Quả Bài Kiểm Tra**:

   - Đối với `ping`:
     - `Packets: X sent, Y received, Z% loss`: Tóm tắt việc gửi và nhận gói tin.
     - `RTT: min=X ms, avg=Y ms, max=Z ms, jitter=W ms`: Các thông số thời gian phản hồi.
     - `Ping.Status`: Kết quả kiểm tra (0 = thành công, 1 = thất bại).
     - Các thông số khác: `Ping.host`, `Ping.hostAddress`, `Ping.successCount`, v.v.
   - Đối với `speedtest`:
     - `No server specified, using default server (nearest)`: Cho biết server mặc định được sử dụng.
     - `Server ID for future reference`: ID của server được dùng.
     - `Speedtest completed successfully`: Bao gồm thông tin server, ISP và các thông số tốc độ.

4. **Lỗi hoặc Cảnh Báo**:

   - Tìm các dòng có `ERROR` hoặc `WARN` để xác định vấn đề (ví dụ: `Failed to execute speedtest command` nếu `speedtest-cli` chưa được cài đặt).

## Khắc Phục Sự Cố

- **Bài Kiểm Tra Không Chạy Được**:

  - Kiểm tra xem các công cụ cần thiết (`speedtest-cli`) đã được cài đặt chưa.
  - Đảm bảo file cấu hình JSON (`config/ping.json`, `config/speedtest.json`) hợp lệ và tồn tại.

- **Không Có Kết Quả Trong Log**:

  - Đảm bảo bài kiểm tra có tham số đầu vào đúng (ví dụ: `host` cho `ping`).
  - Kiểm tra các dòng `ERROR` trong log để tìm nguyên nhân.

- **Lỗi Biên Dịch**:

  - Đảm bảo đã cài đặt tất cả các phụ thuộc (`gcc`, `make`, `libcjson-dev`).
  - Chạy `make clean` rồi `make` để biên dịch lại dự án.

## Mở Rộng Hệ Thống

Để thêm một bài kiểm tra mới:

1. Tạo file cấu hình JSON mới trong `config/` (ví dụ: `new_test.json`).
2. Thêm `instruction` tương ứng vào `config.json`.
3. Viết trình xử lý trong `src/test/` (ví dụ: `new_test.c`) và đăng ký trong `src/action/action_registry.c`.
4. Cập nhật `Makefile` để bao gồm file mã nguồn mới.

