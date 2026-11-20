# STM32 Access Control System

Tài liệu thiết kế hệ thống kiểm soát ra vào (Access Control) sử dụng vi điều khiển
**STM32F103** làm trung tâm xử lý.

## Chức năng chính

- Nhận diện người dùng qua **bàn phím 4x4** (nhập PIN) hoặc **thẻ RFID** (module RC522, 13.56MHz).
- Hiển thị trạng thái/hướng dẫn trên **LCD 16x2**.
- Lưu trữ cấu hình, danh sách PIN/UID thẻ trong **EEPROM 24C256** (I2C, 256Kbit).
- Điều khiển **khóa điện** (lock) qua GPIO + mạch driver.
- **Nút exit** để mở khóa từ phía trong (thoát ra).
- **LED trạng thái** báo hiệu kết quả xác thực / trạng thái hệ thống.
- **UART debug** phục vụ console lệnh (log, cấu hình qua terminal).
- **UART** giao tiếp **module ESP8266** bằng AT command để kết nối Wi-Fi (gửi log/sự kiện lên server, đồng bộ thời gian, OTA...).

## Cấu trúc tài liệu

- [Hardware](hardware/index.md) — kiến trúc phần cứng, sơ đồ khối và chi tiết kết nối từng khối với MCU.
