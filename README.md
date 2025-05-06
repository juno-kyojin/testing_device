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