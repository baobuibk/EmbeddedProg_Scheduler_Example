# Kết nối & Pinout — STM32F103C8T6

Đề xuất gán chân cho LQFP48, tránh các chân SWD (PA13/PA14) và tận dụng chế độ
SWD-only (giải phóng PA15/PB3/PB4 khỏi JTAG).

!!! info "Đã kiểm lại khớp với code thật (mục 1-6, 9), còn mục 7-8 vẫn chỉ là pinout dự kiến"
    Từng chân ở mục 1-6 và 9 đã so khớp trực tiếp với `dio_t { .port, .pin }` thật trong
    `Dev/2.BSP/*.c` (`bsp_keypad`, `bsp_lock`, `bsp_exit_button`, `bsp_rc522`, `bsp_led`) và cấu
    hình I2C1/SPI2 trong `Dev/3.Drivers/Components/{I2C,SPI}/stm32/` — không còn gì lệch. Mục 7
    (UART debug) và 8 (ESP8266) **chưa có dòng code nào** (đúng với phạm vi đã nêu ở
    [use-cases.md](../use-cases.md) — Wi-Fi/UART debug tạm ngoài phạm vi) — pin ở 2 mục đó vẫn
    chỉ là dự kiến, chưa kiểm được gì thật.

## 1. Bàn phím Keypad 4x4

Quét ma trận: 4 chân COL (output, push-pull) quét lần lượt mức thấp, 4 chân ROW
(input, pull-up nội) đọc trạng thái — chiều ngược với bản trước (trước đây ROW
là chân lái, COL là chân đọc). Khớp với driver `Dev/3.Drivers/Devices/keypad4x4/`
(hàm `keypad_col_write()`/`keypad_row_read()`) và `Dev/2.BSP/bsp_keypad.c`.

| Keypad pin | STM32 pin | Chế độ |
|---|---|---|
| COL1 | PA0 | GPIO Output push-pull |
| COL2 | PA1 | GPIO Output push-pull |
| COL3 | PA2 | GPIO Output push-pull |
| COL4 | PA3 | GPIO Output push-pull |
| ROW1 | PA4 | GPIO Input, pull-up |
| ROW2 | PA5 | GPIO Input, pull-up |
| ROW3 | PA6 | GPIO Input, pull-up |
| ROW4 | PA7 | GPIO Input, pull-up |

PA0-PA7 là 8 chân cố định của keypad (khớp driver/BSP) nên **Lock/Exit (mục 3),
ESP8266 (mục 8) và RC522 (mục 6) đã được dời sang các chân khác** để không còn
trùng - xem bảng tổng hợp cuối trang để biết toàn cảnh.

## 2. LCD 16x2 qua I2C (backpack PCF8574)

LCD HD44780 gắn module chuyển đổi I2C (PCF8574/PCF8574A), dùng **chung bus I2C1**
với EEPROM 24C256 — chỉ cần 2 dây SDA/SCL, không chiếm thêm GPIO song song.

!!! info "Từng thử remap I2C1 sang PB8/PB9 để né lỗi mô phỏng — đã đổi lại PB6/PB7"
    PB6/PB7 mặc định cũng là `TIM4_CH1`/`TIM4_CH2` — nghi ngờ là cùng lớp lỗi "logic contention"
    của model `CM3_STM32.DLL` trong Proteus đã gặp với PB12/TIM1_BKIN (CS của RC522, mục 6): xác
    nhận một phần bằng cách comment `bsp_i2c1_init()` trong `bsp_rtc_init()` thì hết crash. Đã thử
    remap sang PB8/PB9 (`AFIO_MAPR.I2C1_REMAP`) nhưng PB8/PB9 cũng là `TIM4_CH3`/`TIM4_CH4` — cùng
    khối TIM4, nên khác RC522, I2C1 không có cặp chân nào tránh hẳn TIM4 trên chip này. Đã đổi lại
    về PB6/PB7 theo yêu cầu; nguyên nhân gốc (nếu đúng là lỗi model) vẫn chưa được xử lý.

| LCD backpack pin | STM32 pin | Ghi chú |
|---|---|---|
| SCL | PB6 (I2C1_SCL) | Chung bus với EEPROM/RTC |
| SDA | PB7 (I2C1_SDA) | Chung bus với EEPROM/RTC |
| VCC | 5V hoặc 3.3V | Tuỳ module (nhiều backpack PCF8574 chạy tốt ở 3.3V, kiểm tra độ sáng backlight nếu dùng 3.3V) |
| GND | GND | |

- **Địa chỉ I2C mặc định:** `0x27` (PCF8574) hoặc `0x3F` (PCF8574A) — có thể đổi qua
  jumper A0/A1/A2 trên backpack nếu cần, miễn không trùng với EEPROM (`0x50`).
- **Contrast (V0)** và **backlight**: đã có biến trở chỉnh contrast và điều khiển
  backlight (qua bit P3 của PCF8574) ngay trên module backpack — không cần mạch
  phụ ở phía STM32.
- Chỉ cần **một** cặp trở pull-up ~4.7kΩ cho toàn bus I2C1 (không lắp lại cho từng thiết bị).

## 3. Khóa điện (Lock) & Nút Exit

Dời khỏi PA0/PA1 (bản trước) vì hai chân đó nay thuộc keypad COL1/COL2 (mục 1).

| Tín hiệu | STM32 pin | Ghi chú |
|---|---|---|
| LOCK_CTRL | PA8 | GPIO Output → mạch driver (relay/MOSFET + opto-isolation) → khóa điện 12V |
| EXIT_BTN | PA11 | GPIO Input, pull-up, active-low — dùng **EXTI11** (ngắt), xem ghi chú debounce dưới |

!!! warning "LOCK_CTRL giả định fail-secure — kiểm tra lại nếu mạch driver thật nối ngược"
    `Dev/2.BSP/bsp_lock.c` giả định: PA8 **LOW** (mặc định lúc reset) = khóa **đóng** (không cấp
    điện cho relay/solenoid); PA8 **HIGH** = relay đóng mạch → khóa **mở**. Đây là kiểu
    fail-secure thường gặp (mất điện = khóa tự đóng lại) — nếu mạch driver/relay thật đi dây theo
    kiểu fail-safe (ngược lại), chỉ cần đổi `do_set()`/`do_reset()` trong `bsp_lock_open()`/
    `bsp_lock_close()`, không cần sửa gì ở `app_lock` hay cao hơn.

!!! info "EXIT_BTN debounce 200ms bằng phần mềm, ở Application chứ không phải trong ISR"
    PA11 cấu hình EXTI11 falling-edge ngắt thật; `EXTI15_10_IRQHandler` (ISR, `Dev/2.BSP/bsp_exit_button.c`)
    không debounce gì (chỉ set 1 cờ, an toàn trong ngắt). Debounce thật nằm ở
    `Dev/1.Application/app_exit_button.c` (`app_exit_button_on_press()`, chạy từ main loop, có
    khái niệm thời gian rõ ràng) — bỏ qua mọi lần nhấn trong vòng 200ms kể từ lần được chấp nhận
    gần nhất. Xem [app-lock.md](../firmware/app-lock.md) để biết chi tiết + lý do đổi so với bản
    trước (từng chủ động không debounce vì cho là không cần).

## 4. EEPROM 24C256 (I2C, chung bus với LCD/RTC)

| EEPROM pin | STM32 pin | Ghi chú |
|---|---|---|
| SCL | PB6 (I2C1_SCL) | Chung bus I2C1 với LCD/RTC (xem mục 2, 5) |
| SDA | PB7 (I2C1_SDA) | Chung bus I2C1 với LCD/RTC (xem mục 2, 5) |
| A0, A1, A2 | GND | Chọn địa chỉ I2C = `0x50`, không trùng LCD (`0x27`/`0x3F`) hay RTC (`0x68`) — nhưng xem cảnh báo về EEPROM onboard của module DS1307 |
| WP | GND | Cho phép ghi (kéo lên VCC nếu muốn khóa ghi phần cứng) |
| VCC | 3.3V | |

## 5. Module RTC DS1307 (I2C, chung bus với EEPROM/LCD)

| DS1307 pin | STM32 pin | Ghi chú |
|---|---|---|
| SCL | PB6 (I2C1_SCL) | Chung bus I2C1 với EEPROM/LCD (xem mục 2, 4) |
| SDA | PB7 (I2C1_SDA) | Chung bus I2C1 với EEPROM/LCD (xem mục 2, 4) |
| VBAT | Pin CR2032 (3V) | Nguồn backup giữ giờ khi mất điện chính — bắt buộc để RTC hoạt động đúng chức năng |
| SQW/OUT | Không bắt buộc nối | Ngõ ra xung vuông (1Hz hoặc tần số dao động), open-drain — chỉ nối (kèm trở pull-up riêng) nếu cần dùng làm nguồn ngắt định kỳ cho STM32 |
| VCC | 3.3V hoặc 5V | DS1307 chấp nhận cả hai; dùng 3.3V để đồng nhất với bus I2C logic 3.3V |
| GND | GND | |

- **Địa chỉ I2C cố định:** `0x68` (không đổi được, theo datasheet DS1307) — không
  trùng với EEPROM (`0x50`) hoặc LCD (`0x27`/`0x3F`).
- Cần thạch anh **32.768kHz** cho DS1307 — trên các module bán sẵn, thạch anh này
  đã được hàn cố định trên board, không cần mạch ngoài.
- Xem cảnh báo về rủi ro trùng địa chỉ `0x50` nếu board DS1307 có kèm EEPROM
  AT24C32 onboard tại [Tổng quan & Sơ đồ khối](index.md).

## 6. Module RFID RC522 (SPI)

Dời từ **SPI1** (PA4-PA7, bản trước) sang **SPI2** vì PA4-PA7 nay thuộc keypad
ROW1-ROW4 (mục 1) - `stm32_spi_io.c` đã hỗ trợ sẵn SPI2 (PB13-PB15, không remap).
SPI1 (PA4-PA7) nhờ vậy được giải phóng hoàn toàn cho dự phòng mở rộng.

| RC522 pin | STM32 pin | Ghi chú |
|---|---|---|
| SCK  | PB13 (SPI2_SCK)  | |
| MISO | PB14 (SPI2_MISO) | |
| MOSI | PB15 (SPI2_MOSI) | |
| SDA (CS/NSS) | PA15 | Điều khiển CS bằng GPIO phần mềm (SPI2 vẫn ở chế độ software-NSS) — xem khung dưới vì sao không phải PB12 |
| RST  | PB0 | GPIO Output, reset module khi cần |
| IRQ  | PB5 | Đi dây sẵn, GPIO Input pull-up — **không dùng ngắt EXTI**, xem ghi chú dưới |
| VCC  | **3.3V** | **Không cấp 5V — module có thể hỏng** |
| GND  | GND | |

!!! tip "Cấu hình chân IRQ"
    Chân IRQ của MFRC522 mặc định là **open-drain, active-low** (bit `IRQPushPull`
    trong `ComIEnReg` quyết định open-drain hay push-pull — mặc định open-drain).
    Vì vậy PB5 cần cấu hình **Input pull-up** (dùng pull-up nội của STM32 là đủ,
    không cần trở ngoài) và bắt **EXTI falling-edge**. Firmware cần bật bit tương
    ứng trong `ComIEnReg`/`DivIEnReg` của RC522 (ví dụ `RxIRq`) để module thực sự
    kéo chân IRQ xuống thấp khi phát hiện thẻ, thay vì phải poll thanh ghi trạng thái.

    Hiện tại **không dùng** EXTI/ngắt cho chân này: `Dev/2.BSP/bsp_rc522` chỉ poll
    (REQA) và đọc UID ngay trong cùng một lời gọi khi REQA thành công
    (`Dev/1.Application/app_rc522`), không qua `Dev/4.MiddleWare/event`. Lý do: bộ
    lập lịch ở đây là cooperative và lệnh REQA đã block đồng bộ chờ SPI xong, nên
    chờ thêm một ngắt EXTI nữa (bắn ra gần như cùng lúc SPI poll trả về) không tận
    dụng được gì - không có task nào khác chạy được trong lúc chờ dù dùng cách
    nào. Chân IRQ vẫn được đi dây trên board, chỉ đáng dùng nếu sau này viết lại
    `rc522_transceive()` theo hướng non-blocking thật (gửi lệnh rồi trả về ngay,
    CPU làm task khác, ngắt báo khi SPI xong).

!!! warning "CS đã dời từ PB12 sang PA15 — chỉ để né lỗi mô phỏng Proteus, không phải lỗi phần cứng"
    Lần gán đầu là PB12 (vẫn hợp lệ 100% về điện - phần cứng thật không có vấn đề gì). Đổi sang
    PA15 vì: PB12 mặc định (không remap) còn là `TIM1_BKIN`, và model Cortex-M3 cụ thể trong
    Proteus đang dùng (`CM3_STM32.DLL`, một model bên thứ 3/không chính thức, khá nổi tiếng kém
    ổn định) báo nhầm **"logic contention"** giữa 2 net ảo của model (`GPIO_B12` và `TIM1_BKIN`)
    rồi **crash (access violation)** khi CS bị toggle nhanh — dù TIM1 chưa bao giờ được cấp clock
    trong firmware (đã kiểm `RCC->APB2ENR`, không có `TIM1EN`) nên không có tranh chấp thật nào
    trên silicon thật. PA15 không có đụng alternate-function nào bị toggle bởi firmware này: nó
    là JTDI khi dùng JTAG đầy đủ, nhưng debug qua **SWD** (ST-Link) không bao giờ chạm PA15, nên
    dùng làm GPIO thường ở đây an toàn mà không cần tắt JTAG-DP qua `AFIO->MAPR`.

## 7. UART Debug Console (USART1)

**Ngoài phạm vi hiện tại** (xem khung cảnh báo "Phạm vi" ở [use-cases.md](../use-cases.md)) — pin
dự kiến, chưa có `bsp_uart`/driver UART nào trong code.

| Tín hiệu | STM32 pin | Ghi chú |
|---|---|---|
| TX | PA9 (USART1_TX) | Nối tới RX của mạch UART-USB (CP2102/CH340/FT232) |
| RX | PA10 (USART1_RX) | Nối tới TX của mạch UART-USB |

## 8. UART ESP8266 (USART3, AT command)

Dời từ **USART2** (PA2/PA3, bản trước) sang **USART3** vì PA2/PA3 nay thuộc
keypad COL3/COL4 (mục 1). STM32F103C8 (medium-density) có USART3 - không có ở
bản C6T6 (low-density) ban đầu.

**Ngoài phạm vi hiện tại** (xem khung cảnh báo "Phạm vi" ở [use-cases.md](../use-cases.md)) — pin
dự kiến, chưa có `bsp_uart`/driver AT-command nào trong code.

| Tín hiệu | STM32 pin | Ghi chú |
|---|---|---|
| TX | PB10 (USART3_TX) | → RX của ESP8266 |
| RX | PB11 (USART3_RX) | ← TX của ESP8266 |
| CH_PD / EN | 3.3V (hoặc GPIO nếu muốn điều khiển bật/tắt module) | Kéo cao để module hoạt động |
| RST | 3.3V qua điện trở (hoặc GPIO nếu muốn reset phần mềm) | Tuỳ chọn |

!!! tip "Không bắt buộc cần level-shifter"
    ESP8266 chạy logic 3.3V, cùng mức với STM32F103 nên có thể nối UART trực tiếp
    chân-chân, không cần mạch chuyển mức.

!!! info "Đánh đổi: chiếm luôn chân của I2C2"
    PB10/PB11 là vị trí **cố định duy nhất** của cả USART3 và I2C2 trên gói
    LQFP48 này (không có remap nào trong 2 khối đó trỏ tới chân khác còn tồn tại
    trên gói 48 chân) - dùng USART3 ở đây nghĩa là **I2C2 sẽ không bao giờ dùng
    được** trên board này. Chấp nhận được vì hiện chưa có thiết bị nào gắn vào
    I2C2 (xem [Tổng quan](index.md)) - nhưng nếu sau này cần I2C2 thật, phải bỏ
    ESP8266 khỏi USART3 trước.

## 9. LED trạng thái

**Đã có code, nhưng chỉ là nhấp nháy 1Hz "còn sống" (heartbeat)** — `bsp_led`/`app_led`
(`app_led_task()`, chu kỳ 1s: toggle mỗi 500ms) hoàn toàn độc lập, không nhận input từ
`app_system`. **Vẫn chưa phải** LED xanh/đỏ theo kết quả xác thực mà UC-02/03/04/05 cần (xem cột
"Trạng thái firmware" trong [use-cases.md](../use-cases.md)) — `app_system` chưa gọi API nào
của `app_led` để báo granted/denied/lockout; đó vẫn là khoảng thiếu, chỉ là bây giờ ít nhất đã có
driver GPIO sẵn để xây tiếp lên trên, thay vì từ 0.

| Tín hiệu | STM32 pin | Ghi chú |
|---|---|---|
| LED_STATUS | PC13 | GPIO Output, qua trở hạn dòng (~330Ω–1kΩ) |

## Tổng hợp sử dụng Port/Peripheral

Đã tổ chức lại để không còn chân nào bị dùng 2 lần (xem ghi chú "Dời từ..." ở
các mục 3/6/8 phía trên).

| Peripheral | Chân dùng |
|---|---|
| Keypad 4x4 (COL output + ROW input, mục 1) | PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7 |
| Lock + Exit (mục 3) | PA8, PA11 |
| USART1 (debug, mục 7) | PA9, PA10 |
| SWD (debug probe) | PA13, PA14 |
| I2C1 (EEPROM + LCD + RTC DS1307, chung bus, mục 2/4/5) | PB6, PB7 |
| RC522 RST/IRQ (mục 6) | PB0, PB5 |
| USART3 (ESP8266, mục 8) | PB10, PB11 |
| SPI2 data (RC522, mục 6) | PB13, PB14, PB15 |
| RC522 CS (mục 6) | PA15 |
| LED trạng thái (mục 9) | PC13 |

Chân còn trống: PA12, PB1-PB4, PB8, PB9, PB12, PC14, PC15 (dự phòng mở rộng -
buzzer, cảm biến cửa, relay thứ 2, LED đa trạng thái...). **PB12** nằm trong nhóm
này dù từng được gán cho CS - xem khung cảnh báo ở mục 6 vì sao đổi sang PA15.
**SPI1 (PA4-PA7) được giải phóng hoàn toàn** sau khi RC522 dời sang SPI2 - không
còn khối nào dùng.

!!! info "I2C2 không dùng được trên board này"
    USART3 (ESP8266, mục 8) chiếm PB10/PB11 - vị trí cố định duy nhất của I2C2
    trên gói LQFP48 này. Đây là đánh đổi đã chấp nhận vì chưa có thiết bị I2C2
    nào trong thiết kế (xem ghi chú ở mục 8).

!!! note "Về số lượng chân khả dụng"
    STM32F103C8T6 (LQFP48) có khoảng 37 chân GPIO lý thuyết. Hệ thống hiện dùng
    **23 chân GPIO phân biệt** (bảng trên, không tính 2 chân SWD) - còn dư ~10
    chân (kể trên) để mở rộng.
