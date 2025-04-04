#!/bin/bash

# Chuyển đến thư mục gốc của dự án
cd "$(dirname "$0")/.." || exit 1

# Đảm bảo scripts có quyền thực thi
chmod +x scripts/testing-service.sh

# Cài đặt service vào systemd
echo "Installing systemd service..."
sudo cp systemd/testing-device.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable testing-device.service

echo "Starting service..."
sudo systemctl start testing-device.service
echo "Service status:"
sudo systemctl status testing-device.service

echo "Installation completed!"
echo "You can now use the following commands to control the service:"
echo "  sudo systemctl start testing-device.service"
echo "  sudo systemctl stop testing-device.service"
echo "  sudo systemctl restart testing-device.service"
echo "  sudo systemctl status testing-device.service"
echo ""
echo "Test cases should be placed in: /home/tobie/testing_device/var/input/"
echo "Results will be available in: /home/tobie/testing_device/var/results/"
echo "Logs can be found in: /home/tobie/testing_device/var/log/"
