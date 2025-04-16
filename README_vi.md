# Hệ thống Plugin cho Testing Device

## Tổng quan

Testing device hỗ trợ các plugin động để mở rộng chức năng. Hệ thống plugin cho phép thêm các loại test mới mà không cần biên dịch lại ứng dụng chính.

## Vấn đề với plugin trùng tên action

Trong hệ thống plugin, mỗi plugin phải đăng ký một "action_name" duy nhất để hệ thống nhận diện. Tuy nhiên, có thể xảy ra tình huống nhiều plugin cùng đăng ký một action. Trong trường hợp này:

1. Plugin đầu tiên được nạp sẽ giữ nguyên action_name
2. Các plugin sau sẽ được đổi tên thành `action_v2`, `action_v3`, v.v.
3. Khi thực thi test, plugin đầu tiên được nạp sẽ được sử dụng, trừ khi cấu hình chỉ định plugin cụ thể

## Cách sử dụng plugin cụ thể

Trong file cấu hình JSON, bạn có thể chỉ định plugin cụ thể bằng cách thêm tham số "plugin" vào phần input_params:

```json
{
  "test_cases": [
    {
      "action": "ping",
      "input_params": {
        "host": "google.com",
        "plugin": "Debug Ping Plugin"  // Tên của plugin cụ thể
      }
    }
  ]
}
```

## Cách tạo plugin mới

1. Sao chép file mẫu `plugins/template_plugin.c`
2. Chỉnh sửa thông tin plugin (tên, mô tả, phiên bản, action_name)
3. Triển khai logic cho hàm execute_test
4. Biên dịch plugin bằng script: `./scripts/compile_plugin.sh my_plugin.c`
5. Đặt file .so vào thư mục plugins

## Quản lý các plugin

Để xem danh sách các plugin đã nạp, kiểm tra log khi chạy ứng dụng:

