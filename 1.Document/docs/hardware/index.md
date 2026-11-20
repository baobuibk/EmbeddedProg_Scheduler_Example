# Hardware — Tổng quan & Sơ đồ khối

## Thành phần phần cứng

| # | Khối | Linh kiện đề xuất | Giao tiếp với MCU |
|---|------|-------------------|--------------------|
| 1 | MCU chính | STM32F103C8T6 (LQFP48, Cortex-M3) | — |
| 2 | Bàn phím | Keypad 4x4 (ma trận) | 8x GPIO (quét ma trận) |
| 3 | Hiển thị | LCD 16x2 I2C (HD44780 + backpack PCF8574) | I2C1 (dùng chung bus với EEPROM) |
| 4 | Khóa | Khóa điện / solenoid lock | 1x GPIO OUT → mạch driver (relay/MOSFET) |
| 5 | Nút thoát | Exit button | 1x GPIO IN |
| 6 | Lưu trữ | EEPROM 24C256 (I2C, 256Kbit) | I2C1 (SCL/SDA) |
| 7 | RFID | Module đọc thẻ RC522 (MFRC522, 13.56MHz) | SPI2 (SCK/MISO/MOSI/CS) + GPIO (RST) + GPIO (IRQ, đi dây sẵn nhưng firmware hiện chỉ polling — xem [Kết nối & Pinout](connections.md#6-module-rfid-rc522-spi)) |
| 8 | Debug | UART-USB bridge (CP2102/CH340...) | USART1 (console lệnh) |
| 9 | Kết nối mạng | Module ESP8266 (AT command) | USART2 |
| 10 | Trạng thái | LED status | 1x GPIO OUT |
| 11 | Thời gian thực | Module RTC DS1307 (+ pin CR2032) | I2C1 (dùng chung bus với EEPROM/LCD) |

!!! note "Giả định thiết kế"
    - MCU dùng dao động nội **HSI** (không cần thạch anh ngoài), khớp với cấu hình
      `SystemClock_Config()` hiện tại trong firmware (`Firmware/Platform/RFID/Core/Src/main.c`).
    - Debug dùng **SWD** (không dùng JTAG) để giải phóng các chân PA15, PB3, PB4 cho
      mục đích khác (ví dụ quét keypad).

## Sơ đồ khối hệ thống

```mermaid
graph TB
    subgraph PWR["Khối nguồn"]
        ADP["Adapter 12V DC"] --> BUCK["Buck/LDO 12V to 3.3V"]
        ADP --> LOCKPWR["Nguồn 12V cho khóa"]
    end

    MCU["STM32F103C8T6"]

    KEYPAD["Keypad 4x4"] <-->|"8x GPIO
    4 ROW out + 4 COL in"| MCU
    BTN["Exit Button"] -->|"1x GPIO IN"| MCU
    LED["Status LED"] <--|"1x GPIO OUT"| MCU
    MCU -->|"1x GPIO OUT"| DRV["Relay / MOSFET Driver
    + Opto-isolation"]
    DRV --> LOCK["Khóa điện / Solenoid Lock"]

    MCU <-->|"I2C1 (shared bus)
    SCL / SDA"| EEPROM["EEPROM 24C256
    (addr 0x50)"]
    MCU <-->|"I2C1 (shared bus)"| LCD["LCD 16x2 I2C
    PCF8574 (addr 0x27)"]
    MCU <-->|"I2C1 (shared bus)"| RTC["Module RTC DS1307
    (addr 0x68) + CR2032"]
    MCU <-->|"SPI2
    SCK/MISO/MOSI/CS + RST"| RFID["Module RC522 (RFID 13.56MHz)"]
    RFID -.->|"IRQ (GPIO, đi dây sẵn —
    firmware hiện chỉ polling, chưa dùng ngắt)"| MCU
    MCU <-->|"USART1
    TX/RX"| DEBUG["USB-UART Debug Console"]
    MCU <-->|"USART2
    AT Command"| ESP["Module ESP8266 (Wi-Fi)"]

    BUCK -.->|"3.3V"| MCU
    BUCK -.->|"3.3V"| EEPROM
    BUCK -.->|"3.3V"| LCD
    BUCK -.->|"3.3V"| RTC
    BUCK -.->|"3.3V"| RFID
    LOCKPWR -.->|"12V"| DRV
```

## Lưu ý kiến trúc quan trọng

!!! warning "RC522 chỉ hoạt động ở 3.3V"
    Module RC522 **không tương thích 5V** (cả nguồn và tín hiệu). Vì STM32F103 cũng
    là logic 3.3V nên kết nối trực tiếp không cần level shifter — chỉ cần đảm bảo
    cấp nguồn 3.3V ổn định, dòng đủ (có thời điểm module kéo dòng tăng vọt khi đọc thẻ).

!!! warning "ESP8266 cần nguồn riêng, dòng đủ lớn"
    ESP8266 có thể kéo dòng đỉnh 200-300mA khi truyền Wi-Fi, dễ gây sụt áp nếu dùng
    chung LDO yếu với MCU. Nên dùng LDO 3.3V riêng (≥500mA) cho ESP8266, tụ lọc
    nguồn (≥100µF) gần chân VCC module.

!!! danger "Cách ly mạch điều khiển khóa"
    Khóa điện (solenoid/electric strike) thường dùng điện 12V và dòng cảm kháng lớn.
    **Không** nối trực tiếp GPIO vào cuộn hút khóa. Dùng relay module hoặc MOSFET +
    diode flyback (chống xung ngược), lý tưởng có opto-isolation giữa GPIO và tầng
    công suất để bảo vệ MCU.

!!! info "LCD, EEPROM và RTC dùng chung bus I2C1"
    LCD qua backpack **PCF8574** (`0x27`), EEPROM **24C256** (`0x50`) và RTC
    **DS1307** (`0x68`, địa chỉ cố định theo datasheet) không trùng nhau nên cả ba
    đi chung một bus I2C1 (PB6/PB7), chỉ cần **một** cặp trở pull-up 4.7kΩ cho cả
    bus — giúp giải phóng 6 chân GPIO (trước đây dùng cho LCD parallel) để dự
    phòng mở rộng (buzzer, cảm biến cửa...).

!!! danger "Rủi ro trùng địa chỉ nếu dùng board DS1307 có sẵn EEPROM onboard"
    Nhiều module DS1307 bán sẵn (ví dụ loại "Tiny RTC") tích hợp luôn một chip
    **EEPROM AT24C32 ở địa chỉ I2C `0x50`** trên cùng board — **trùng thẳng** với
    EEPROM 24C256 đã có trong thiết kế này. Trước khi lắp, kiểm tra kỹ:

    - Nếu board DS1307 có AT24C32 onboard: **không kết nối chân SDA/SCL của
      AT24C32 đó vào bus chung** (nhiều board có pad/jumper để cắt bỏ phần EEPROM
      này), hoặc đổi địa chỉ EEPROM 24C256 rời qua chân A0/A1/A2 (datasheet 24C256
      có hỗ trợ) để tránh trùng `0x50`.
    - Nếu dùng IC DS1307 rời (không kèm EEPROM) thì không có vấn đề này.
    - DS1307 cần nguồn backup **CR2032 (pin 3V)** nối vào chân VBAT để giữ giờ khi
      mất điện chính — hầu hết module đã có sẵn đế pin này trên board.

Xem chi tiết pinout và bảng kết nối từng khối tại [Kết nối & Pinout](connections.md).
