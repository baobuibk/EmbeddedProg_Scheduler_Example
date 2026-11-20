# Module đã viết & cấu trúc dữ liệu

Trạng thái tính đến tài liệu này: tất cả build sạch bằng `Firmware/Makefile`. Mọi Device driver
trong dự án (`keypad4x4`, `ds1307`, `eeprom24cxx`, `lcd1602_i2c`, `rc522`) giờ đã có BSP/App thật
nối tới và đang chạy (có trong `g_pTaskTable`/`g_pEventTable`, xác nhận bằng
`arm-none-eabi-nm` - chỉ `ds3234` còn là code tham khảo không dùng). `app_system` là lớp điều
phối trung tâm - máy trạng thái đầy đủ nối `app_rc522`/`app_keypad`/`app_persist`/`app_rtc` →
`app_lock` + `app_lcd` + `app_log` (UC-02/05/06/07/08/09) — xem [app-system.md](app-system.md)
để có sơ đồ máy trạng thái đầy đủ, [app-lock.md](app-lock.md) cho máy trạng thái riêng của khóa,
và [app-log.md](app-log.md) cho ring buffer ghi log.

## 5.Utils — mã lỗi chung

`define.h` — không có struct, chỉ định nghĩa mã lỗi `uint32_t` dùng xuyên suốt mọi lớp:

```c
#define ERROR_OK                (0u)
#define ERROR_FAIL              (1u)
#define ERROR_INVALID_PARAM     (2u)
#define ERROR_TIMEOUT           (3u)
#define ERROR_BUSY              (4u)
#define ERROR_SPI               (5u)
#define ERROR_NACK              (6u)    /* I2C: address/data không được ACK */
```

## 4.MiddleWare

### scheduler

Bộ lập lịch hợp tác (cooperative), không ưu tiên (no preemption) — mô phỏng theo thư viện
scheduler của TI (bản dịch tiếng Việt vẫn còn trong comment gốc).

```c
typedef void (*tSchedulerFunction)(void);

typedef struct {
    void (*pfnFunction)();   // hàm được gọi định kỳ
    uint32_t ui32FrequencyTicks; // 0 = gọi mỗi lần scheduler_run()
    uint32_t ui32LastCall;       // scheduler tự cập nhật
    bool bActive;
} tSchedulerTask;

extern tSchedulerTask g_pTaskTable[];     // client (app_main.c) phải định nghĩa
extern uint32_t g_ui32SchedulerNumTasks;  // số phần tử của bảng trên
```

API chính: `scheduler_init(ui32TicksPerSecond)`, `scheduler_run()`, `scheduler_task_enable/disable()`,
`scheduler_tick_count_get()`, `scheduler_elapsed_ticks_get/calc()`. Hook platform (weak, override
bởi `bsp_systick`): `void platform_systick_init(uint32_t TicksPerSecond)`.

### event

Cơ chế **tách riêng, bổ sung** cho scheduler — chỉ dành cho những gì cần phản ứng theo *ngắt*
(bất đồng bộ) thay vì theo *thời gian*. `scheduler` không bao giờ nhận biết ngắt; module này cũng
không bao giờ biết lịch chạy — cố tình không trộn hai khái niệm vào một bảng.

```c
typedef void (*tEventHandler)(void);

typedef struct {
    tEventHandler pfnHandler;
    volatile bool bPending;  // set bởi event_post() — an toàn gọi từ ISR
} tEventEntry;

extern tEventEntry g_pEventTable[];     // client (app_main.c) phải định nghĩa
extern uint32_t g_ui32EventNumEvents;   // số phần tử của bảng trên
```

API: `event_post(ui32EventId)` (chỉ set 1 cờ `volatile`, an toàn gọi trong ISR), `event_run()`
(duyệt bảng, gọi handler của mọi sự kiện đang `bPending`, xoá cờ *trước* khi gọi — một `event_post`
mới xảy ra đúng lúc handler chạy không bị mất, chỉ dời sang lần `event_run()` kế). Cùng kiểu sở
hữu bảng với `scheduler`'s `g_pTaskTable` — ID của 1 sự kiện là index của nó trong `g_pEventTable`.

!!! note "RC522 thử dùng rồi bỏ — nút exit là consumer thật đầu tiên"
    RC522 từng dùng module này (ngắt EXTI trên chân IRQ của RC522) rồi **bỏ**: scheduler ở đây là
    cooperative và lệnh REQA của RC522 đã block đồng bộ chờ SPI xong, nên một ngắt thêm không tận
    dụng được gì — không task nào chạy được "trong lúc chờ" dù theo cách nào (xem
    [rc522](#rc522-rfid-mfrc522-spi) và [app_rc522](#app_rc522)).

    `g_pEventTable[1]` giờ có **1 entry thật**: `bsp_exit_button`'s `EXTI15_10_IRQHandler` post
    `BSP_EXIT_BUTTON_EVENT_PRESSED` khi nhấn nút exit, `app_exit_button_on_press()` xử lý — xem
    [app-lock.md](app-lock.md). Khác RC522: nút nhấn là input rời rạc thật, không cần "hỏi" định
    kỳ, nên EXTI ở đây thực sự thay được polling.

### os (backend: baremetal)

Trừu tượng hoá khoá + chờ cờ thanh ghi, để driver không phụ thuộc trực tiếp HAL/RTOS.

```c
typedef struct {
    volatile uint8_t locked;
} os_lock_t;
```

API: `os_lock_init/acquire/release()`, `os_reg_wait_flag_blocking/clear_blocking()` (chờ bit set/clear
trên 1 thanh ghi, có timeout), `os_tick_get()`. Backend `baremetal` dùng tick của chính
`scheduler` (`scheduler_tick_count_get()`) — `os_lock_acquire()` *spin* (không có RTOS để chuyển task).

## 3.Drivers/Components — trừu tượng hoá bus (không biết board/chip)

### dio

```c
typedef struct {
    uint32_t port;   // 1=GPIOA, 2=GPIOB, 3=GPIOC (backend stm32)
    uint32_t pin;    // 0-15
    bool bStatus;    // backend tự cập nhật sau mỗi lần set/reset/toggle/read
} dio_t;

typedef enum { INPUT = 0, INPUT_PULLUP, INPUT_PULLDOWN, OUTPUT } dio_mode_t;
```

API: `do_set/do_reset/do_toggle()`, `dio_mode()`, `di_read()`, `dio_flow_address_pulse()` (xung
200µs dùng DWT cycle-counter, cho cảm biến SLF3x — hiện không dùng trong hệ thống này).

!!! info "Vì sao không tên `do_t`"
    Trước đây type này tên `do_t` ("digital output"), nhưng cùng 1 instance có thể cấu hình làm
    input (`dio_mode(INPUT_PULLUP)` + `di_read()`) hay output (`dio_mode(OUTPUT)` + `do_set()`) —
    "do" (chỉ output) gây hiểu nhầm, nên đã đổi thành `dio_t`/`dio.h`. Tên hàm riêng lẻ
    (`do_set`, `di_read`...) giữ nguyên vì mỗi hàm đúng là chỉ làm 1 chiều.

Backend đang build: `stm32/stm32_dio.c`. Backend tham khảo (loại khỏi build qua
`DEV_EXCLUDE_DIRS` trong Makefile): `dummy/dummy_dio.c` (weak stub), `nxp/dio.c`,
`samv71/samv71_dio.c`.

!!! warning "2 backend tham khảo bị lộn nội dung so với tên thư mục"
    File vật lý trong `dio/nxp/` thực ra là code Microchip/Atmel SAMV71 (PIO/PMC), còn file
    trong `dio/samv71/` thực ra là code NXP (RGPIO). Không ảnh hưởng build thật (cả hai bị loại
    khỏi Makefile), chỉ gây nhầm khi đọc — chưa sửa.

### i2c_io

```c
typedef struct {
    uint32_t   ui32I2cPort;  // 1=I2C1, 2=I2C2 (backend stm32, STM32F103C8 medium-density)
    os_lock_t  lock;
} i2c_io_t;
```

API: `i2c_io_init()`, `i2c_io_set_speed()`, `i2c_io_lock/unlock()`, `i2c_io_read_sync/write_sync/
write_read_sync()` (sync); các hàm async/DMA đã khai báo trong header nhưng **chưa implement**
ở backend stm32.

!!! danger "Chính sách khoá — đọc trước khi dùng"
    Không hàm `i2c_io_*` nào tự khoá `me->lock`. Nơi gọi phải tự `i2c_io_lock()` trước và
    `i2c_io_unlock()` sau mỗi giao dịch (dù chỉ 1 lệnh). Và quan trọng hơn: **1 bus vật lý chỉ
    được có đúng 1 `i2c_io_t`** — xem `bsp_i2c1` bên dưới để biết lý do.

Backend đang build: `stm32/stm32_i2c_io.c` (I2C1: PB6/PB7; I2C2: PB10/PB11 — nhưng PB10/PB11
hiện đã bị USART3/ESP8266 chiếm, xem [Kết nối & Pinout](../hardware/connections.md#8-uart-esp8266-usart3-at-command)
— I2C2 **không dùng được thật** trên board này).

!!! info "Backend này dùng LL (Low-Layer), không phải thanh ghi CMSIS thủ công"
    Khác với hầu hết backend khác trong `3.Drivers/Components` (SPI, dio — vẫn thao tác thanh ghi
    CMSIS trực tiếp), `stm32_i2c_io.c` đã viết lại dùng API `LL_I2C_*`/`LL_GPIO_*`/`LL_RCC_*`
    (`stm32f1xx_ll_i2c.h` v.v., cần define `USE_FULL_LL_DRIVER` — đã thêm vào `Makefile`). Lý do:
    cả project đã chuyển từ HAL sang LL (phần `Core/`/`.ioc` do người dùng tự chuyển), nên
    `-DUSE_HAL_DRIVER` không còn hợp lệ — đã bỏ khỏi `Makefile`, thay bằng `-DUSE_FULL_LL_DRIVER`
    (cần cho các struct `LL_I2C_InitTypeDef`/hàm `LL_I2C_Init()` mà `main.c` tự viết dùng). API
    public của `i2c_io.h` không đổi gì — mọi nơi gọi (`bsp_i2c1`, `bsp_rtc`, `bsp_lcd`,
    `bsp_eeprom`) không cần sửa.

### spi_io

```c
typedef enum { SPI_MODE_0 = 0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3 } spi_mode_t;
typedef enum { SPI_BIT_ORDER_MSB_FIRST = 0, SPI_BIT_ORDER_LSB_FIRST } spi_bit_order_t;
typedef enum { SPI_SINGLE_LANE = 0, SPI_DUAL_LANE, SPI_QUAD_LANE, SPI_OCTO_LANE } spi_lanes_t;

typedef struct {
    uint32_t   ui32SpiPort;  // 1=SPI1, 2=SPI2 (backend stm32, STM32F103C8 medium-density)
    os_lock_t  lock;
} spi_io_t;
```

API: `spi_io_init()`, `spi_io_set_frame_size/mode/prescale()`, `spi_io_lock/unlock()`,
`spi_io_read_sync/write_sync/transfer_sync/write_and_read_sync()`. Cùng chính sách khoá như
`i2c_io` — không tự khoá, CS cũng không tự lái (caller/Device tự điều khiển GPIO CS riêng,
SPI luôn ở software-NSS).

Backend đang build: `stm32/stm32_spi_io.c` (SPI1: PA5/PA6/PA7; SPI2: PB13/PB14/PB15 — không
remap). Backend tham khảo (loại khỏi build): `samv71/samv71_spi_io.c`.

## 3.Drivers/Devices — driver theo từng chip

### keypad4x4

Bàn phím ma trận 4x4. **Không nhận pin qua init struct** — thay vào đó khai báo 2 hàm `weak`,
BSP override bằng bản thật (xem `bsp_keypad` bên dưới).

```c
typedef struct {
    char    cLastRawKey;
    uint8_t ui8DebounceCount;
    bool    bKeyReported;   // chống báo lại cùng 1 lần nhấn khi phím còn giữ
} keypad4x4_t;

/* Khai báo weak trong keypad4x4.c, override strong trong bsp_keypad.c */
void keypad_col_write(uint8_t ui8Col, bool bLevel);
bool keypad_row_read(uint8_t ui8Row);
```

API: `keypad4x4_init()`, `keypad4x4_scan_raw()` (không debounce), `keypad4x4_scan()` (debounce,
one-shot — trả ký tự đúng 1 lần mỗi lần nhấn thật). Chỉ hỗ trợ phím đơn — chưa có multi-tap kiểu
Nokia (A-z).

### ds1307 — RTC (I2C, đang dùng thật)

```c
typedef struct {
    uint8_t  ui8Seconds, ui8Minutes, ui8Hours;  // 24h only
    uint8_t  ui8Day, ui8Date, ui8Month;
    uint16_t ui16Year;
} ds1307_time_t;

typedef struct { i2c_io_t *pI2cBus; } ds1307_init_t;
typedef struct { i2c_io_t *pI2cBus; } ds1307_t;
```

API: `ds1307_init()` (tự xoá bit CH/clock-halt nếu oscillator đang dừng), `ds1307_read/write_register()`,
`ds1307_set/get_time()`, `ds1307_is_running/start/stop()`. Không expose thanh ghi điều khiển
SQW/OUT (board này không nối chân đó).

### eeprom24cxx — EEPROM 24C256 (I2C)

```c
typedef struct {
    i2c_io_t *pI2cBus;
    uint16_t  ui16DevAddr;        // 0x50 cho 24C256
    uint32_t  ui32PageSizeBytes;  // 64 cho 24C256
    uint32_t  ui32SizeBytes;      // 32768 cho 24C256
} eeprom24cxx_init_t;  // eeprom24cxx_t: cùng 4 trường
```

API: `eeprom24cxx_init()`, `eeprom24cxx_write/read_byte()`, `eeprom24cxx_write/read()` (buffer
nhiều byte, tự chia theo trang + chờ tWR bằng ACK-polling thay vì busy-wait cố định). BSP/App thật
đã nối (`bsp_eeprom` + `app_persist` - UID/admin password), xem [app-persist.md](app-persist.md).

### lcd1602_i2c — LCD 16x2 qua PCF8574 (I2C)

```c
typedef struct {
    i2c_io_t *pI2cBus;
    uint16_t  ui16DevAddr;   // 0x27 (PCF8574) hoặc 0x3F (PCF8574A)
} lcd1602_init_t;

typedef struct {
    i2c_io_t *pI2cBus;
    uint16_t  ui16DevAddr;
    bool      bBacklightOn;
} lcd1602_t;
```

API: `lcd1602_init()` (chạy sequence khởi tạo HD44780 4-bit chuẩn), `lcd1602_clear/home()`,
`lcd1602_set_cursor()`, `lcd1602_write_char/print()`, `lcd1602_backlight()`,
`lcd1602_display_on_off()`, cấp thấp `lcd1602_command/data()`. BSP/App thật đã nối
(`bsp_lcd` + `app_lcd`), xem [app-system.md](app-system.md).

### rc522 — RFID MFRC522 (SPI)

```c
#define RC522_UID_LEN  4   // chỉ hỗ trợ cascade level 1 (UID 4 byte)

typedef struct {
    spi_io_t *pSpiBus;
    dio_t    *pCsPin;
    dio_t    *pRstPin;
} rc522_init_t;  // rc522_t: cùng 3 trường
```

API: `rc522_init()`, `rc522_read/write_register()`, `rc522_antenna_on/off()`, `rc522_request_a()`
(REQA, phát hiện thẻ), `rc522_read_uid()` (REQA + anticollision cấp 1, kiểm BCC), `rc522_halt()`.
Chỉ polling (đọc `ComIrqReg` qua SPI), **không dùng ngắt IRQ/EXTI** — cố tình, xem [event](#event)
và [app_rc522](#app_rc522) để biết lý do. BSP/Application thật đã nối tới, xem [bsp_rc522](#bsp_rc522).

### ds3234 (tham khảo, không dùng)

```c
typedef struct {
    uint8_t seconds, minutes, hours, day, date, month;
    uint16_t year;
    uint8_t is_pm;
} DS3234_Time_t;

typedef struct {
    spi_io_t *spiBus;
    dio_t    *csPin;
    uint8_t   hourMode, ui8OutputMode, ui8SqwFreq;
} DS3234_Init_t;  // DS3234_Dev_t: thêm trường `initialized`
```

!!! warning "Không phải RTC thật của hệ thống này"
    `ds3234` là RTC qua **SPI** — hệ thống thật dùng **DS1307 qua I2C** (xem
    [Kết nối & Pinout](../hardware/connections.md#5-module-rtc-ds1307-i2c-chung-bus-voi-eepromlcd)).
    File này có vẻ là code mẫu/bài tập cũ (quy ước `PascalCase` khác hẳn phần còn lại của dự án),
    giữ lại nhưng không có BSP/Application nào gọi tới.

## 2.BSP — gắn Device vào board thật

### bsp_i2c1

Sở hữu **đúng 1** `i2c_io_t` cho bus I2C1 — lý do bắt buộc phải là singleton: xem cảnh báo ở
mục [i2c_io](#i2c_io) phía trên.

```c
void bsp_i2c1_init(void);        // idempotent — mọi BSP cần I2C1 cứ tự gọi
i2c_io_t *bsp_i2c1_get(void);    // không bao giờ NULL sau lần init đầu
```

### bsp_rtc

Wrapper mỏng quanh `ds1307` — không cache, mỗi lần gọi là 1 giao dịch I2C thật.

```c
void bsp_rtc_init(void);  // gọi bsp_i2c1_init() + ds1307_init()
uint32_t bsp_rtc_get_time(ds1307_time_t *pTime);
uint32_t bsp_rtc_set_time(const ds1307_time_t *pTime);
```

### bsp_rc522

Sở hữu `spi_io_t` cho SPI2 trực tiếp (không tách `bsp_spi2` riêng như `bsp_i2c1` — SPI2 hiện chỉ
có 1 consumer; tách ra nếu có consumer thứ 2) + 2 `dio_t` (CS=PA15, RST=PB0 — CS từng là PB12,
dời sang PA15 để né 1 lỗi riêng của model STM32 trong Proteus, xem
[connections.md](../hardware/connections.md#6-module-rfid-rc522-spi)). **Thuần polling**, không
cấu hình EXTI/NVIC cho chân IRQ (PB5) — xem [event](#event) để biết lý do.

```c
void bsp_rc522_init(void);
uint32_t bsp_rc522_poll(void);                                            // REQA, rẻ
uint32_t bsp_rc522_read_uid(uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak); // nặng hơn
uint32_t bsp_rc522_halt(void);
```

### bsp_lock

Không có struct public — sở hữu 1 `dio_t` (PA8, output). Giả định polarity **fail-secure**
(mất điện = khóa đóng) — xem ghi chú trong `bsp_lock.h` nếu relay/driver thật nối ngược.

```c
void bsp_lock_init(void);
void bsp_lock_open(void);
void bsp_lock_close(void);
```

### bsp_led

Không có struct public — sở hữu 1 `dio_t` (PC13, output). Giả định polarity **active-HIGH**
(HIGH = sáng) — xem ghi chú trong `bsp_led.h` nếu board có LED onboard kiểu active-low (ví dụ
Blue Pill).

```c
void bsp_led_init(void);
void bsp_led_on(void);
void bsp_led_off(void);
void bsp_led_toggle(void);
```

### bsp_exit_button

Không có struct public — sở hữu 1 `dio_t` (PA11, input pull-up) + cấu hình EXTI11 falling-edge/
NVIC trực tiếp qua thanh ghi CMSIS (cùng cách làm với `bsp_systick`). `EXTI15_10_IRQHandler` post
`BSP_EXIT_BUTTON_EVENT_PRESSED` — xem [event](#event) và [app-lock.md](app-lock.md).

```c
#define BSP_EXIT_BUTTON_EVENT_PRESSED (0U)  // index vào g_pEventTable, dùng cả 2 phía post/đặt bảng

void bsp_exit_button_init(void);
```

### bsp_lcd

Wrapper mỏng quanh `lcd1602_i2c` trên bus `bsp_i2c1` dùng chung (addr `0x27`) — cùng khuôn với
`bsp_rtc`/`bsp_eeprom`. `bsp_lcd_set_line()` là nguyên hàm vẽ duy nhất: luôn clip/pad đúng 16 ký
tự trước khi in, để không bao giờ để lại ký tự cũ thừa ra từ dòng trước dài hơn.

```c
void bsp_lcd_init(void);
void bsp_lcd_set_line(uint8_t ui8Row, const char *pszText);  // ui8Row: 0 hoặc 1
```

### bsp_keypad

Không có struct public — sở hữu 2 mảng `dio_t[4]` static (COL = PA0-3 output, ROW = PA4-7
input pull-up, xem [Kết nối & Pinout](../hardware/connections.md#1-ban-phim-keypad-4x4)) và cung
cấp bản `strong` của 2 hàm `weak` mà `keypad4x4` khai báo:

```c
void bsp_keypad_init(void);
void keypad_col_write(uint8_t ui8Col, bool bLevel);  // override
bool keypad_row_read(uint8_t ui8Row);                // override
```

### bsp_systick

Không có struct, không hàm riêng để gọi — chỉ override `platform_systick_init()` (weak, khai
báo trong `scheduler.h`) để cấu hình SysTick thật theo tốc độ scheduler yêu cầu.

## 1.Application — logic nghiệp vụ

### app_main

Điểm vào duy nhất, không có struct riêng — định nghĩa 2 global bắt buộc của scheduler:

```c
tSchedulerTask g_pTaskTable[7] = {
    { &app_keypad_task, 20,   0, true },  // mỗi 20ms
    { &app_rtc_task,     1000, 0, true }, // mỗi 1000ms (1s)
    { &app_rc522_task,   200,  0, true }, // mỗi 200ms - chỉ REQA, xem app_rc522
    { &app_system_task,  100,  0, true }, // mỗi 100ms - máy trạng thái trung tâm, xem app_system
    { &app_lock_task,    100,  0, true }, // mỗi 100ms - chỉ kiểm hết 5s mở khóa chưa
    { &app_lcd_task,      100,  0, true }, // mỗi 100ms - vẽ lại LCD nếu app_system đổi nội dung
    { &app_led_task,      500,  0, true }, // mỗi 500ms - toggle, 2 lần/chu kỳ = nhấp nháy 1s
};
uint32_t g_ui32SchedulerNumTasks = 7;

tEventEntry g_pEventTable[1] = {
    [BSP_EXIT_BUTTON_EVENT_PRESSED] = { &app_exit_button_on_press, false }
};
uint32_t g_ui32EventNumEvents = 1U;

void app_main(void);  // app_xxx_init() từng module -> scheduler_init(1000) -> for(;;) { scheduler_run(); event_run(); }
```

### app_keypad

```c
void app_keypad_init(void);
void app_keypad_task(void);              // task scheduler, 20ms
bool app_keypad_get_key(char *pcKey);    // đọc 1 lần rồi xoá (one-shot)
```

State nội bộ (static, không public): 1 `keypad4x4_t` + 1 `char` giữ phím đang chờ đọc. Chỉ hỗ
trợ 1 consumer đọc được (gọi 2 lần liên tiếp mà không có phím mới sẽ nhận `false` ở lần 2) — đủ
cho nhu cầu hiện tại (1 nguồn input, chưa có nơi tiêu thụ thứ hai).

### app_rtc

```c
void app_rtc_init(void);
void app_rtc_task(void);   // task scheduler, 1000ms — đọc DS1307 qua bsp_rtc

bool app_rtc_get_time(ds1307_time_t *pTime);
bool app_rtc_consume_if_updated(ds1307_time_t *pTime, uint32_t *pui32LastSeenSeq);
```

**Đây là nơi lưu thời gian của toàn hệ thống**: 1 `static ds1307_time_t` cache ngay trong
`app_rtc.c` (dùng lại thẳng struct của driver `ds1307`, không bọc thêm type mới vì các trường
giống hệt nhau), cùng 1 `static uint32_t` bộ đếm phiên bản (tăng mỗi lần đọc RTC thành công).

Lý do cache ở đây thay vì để mỗi consumer tự gọi `bsp_rtc_get_time()`: I2C1 là bus **chung và
chậm** (EEPROM, LCD cũng trên đó) — tập trung đọc 1 lần/giây ở `app_rtc` tránh mỗi consumer tự
poll chip theo lịch riêng, gây dư giao dịch bus.

Hai API ứng với 2 kiểu tiêu thụ khác nhau:

| API | Dùng cho | Hành vi |
|---|---|---|
| `app_rtc_get_time()` | Log EEPROM (phản ứng theo sự kiện, không theo lịch) | Luôn trả bản mới nhất đang cache |
| `app_rtc_consume_if_updated()` | Hiển thị LCD (chỉ vẽ lại khi *thực sự* đổi) | Mỗi caller tự giữ 1 `uint32_t` "lần cuối mình thấy phiên bản nào" (khởi tạo 0), chỉ nhận dữ liệu + `true` khi phiên bản đã tăng so với lần gọi trước — nhiều consumer độc lập không đụng nhau vì mỗi bên tự so với biến riêng của mình |

`app_rtc_consume_if_updated()` nay có consumer thật: `app_system_tick_idle()` gọi nó mỗi 100ms,
chỉ vẽ lại giờ lên LCD khi giây thực sự đổi (xem [app-system.md](app-system.md)).

!!! note "Chưa xây — là việc tiếp theo"
    Module ghi log EEPROM dạng ring-buffer (sẽ gọi `app_rtc_get_time()`, theo UC-09) chưa được
    viết.

### app_rc522

```c
void app_rc522_init(void);
void app_rc522_task(void);   // task scheduler, 200ms — poll + đọc UID trong cùng 1 lần gọi

bool app_rc522_get_last_uid(uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak);
bool app_rc522_consume_if_updated(uint8_t pui8Uid[RC522_UID_LEN], uint32_t *pui32LastSeenSeq);
```

`app_rc522_task()` gọi `bsp_rc522_poll()` (REQA); chỉ khi thành công mới gọi tiếp
`bsp_rc522_read_uid()` ngay trong cùng lần gọi đó (không qua `event`, xem [event](#event) để biết
lý do), rồi `bsp_rc522_halt()` để thẻ không tự trigger đọc lại liên tục khi còn nằm trong vùng từ
trường. Cache UID + bộ đếm phiên bản theo **đúng pattern của [app_rtc](#app_rtc)**:

| API | Dùng cho | Hành vi |
|---|---|---|
| `app_rc522_get_last_uid()` | Kiểm tra quyền truy cập (phản ứng theo sự kiện) | Luôn trả UID mới nhất đang cache |
| `app_rc522_consume_if_updated()` | Log EEPROM (ghi đúng 1 lần mỗi lượt quẹt thẻ) | Mỗi caller tự giữ 1 `uint32_t` phiên bản riêng, giống `app_rtc_consume_if_updated()` |

Việc so khớp UID với danh sách hợp lệ (trước đây ghi "chưa xây") nay do `app_system` làm —
xem ngay dưới đây.

### app_persist

Nơi lưu **duy nhất** cho mật khẩu admin (UC-08) và danh sách UID + tên (UC-02/06/07) trong
EEPROM, qua `bsp_eeprom`. Nạp 1 lần vào RAM lúc init, mọi lần đọc lấy thẳng từ RAM; mỗi lần sửa
chỉ ghi **đúng vùng đổi** xuống EEPROM (không ghi lại cả blob). Chi tiết đầy đủ (byte layout, sơ
đồ luồng add/remove, lý do thiết kế): xem [app-persist.md](app-persist.md).

```c
#define APP_PERSIST_MAX_USERS           (100u)
#define APP_PERSIST_NAME_LEN            (16u)
#define APP_PERSIST_ADMIN_PASSWORD_LEN  (9u)
#define APP_PERSIST_EEPROM_SIZE_BYTES  /* tính từ 3 hằng số trên - offset app_log bắt đầu */ \
    (4u + 1u + APP_PERSIST_ADMIN_PASSWORD_LEN + 1u + \
     (APP_PERSIST_MAX_USERS * (RC522_UID_LEN + APP_PERSIST_NAME_LEN + 1u)) + 2u)

void app_persist_init(void);
void app_persist_factory_reset(void);
bool app_persist_admin_password_check(const char *pszPassword);
uint32_t app_persist_admin_password_set(const char *pszNewPassword);
uint32_t app_persist_user_add(const uint8_t pui8Uid[RC522_UID_LEN], const char *pszName);
uint32_t app_persist_user_remove(const uint8_t pui8Uid[RC522_UID_LEN]);
bool app_persist_user_find_by_uid(const uint8_t pui8Uid[RC522_UID_LEN], char *pszNameOut);
uint32_t app_persist_user_count(void);
bool app_persist_user_get_by_index(uint32_t ui32Index, uint8_t pui8Uid[RC522_UID_LEN], char *pszNameOut);
```

### app_lock

Máy trạng thái 2 trạng thái (`LOCKED`/`UNLOCKED`) điều khiển khóa qua `bsp_lock`. **Không biết
gì về RC522/nút exit/bàn phím** — mọi nơi khác chỉ "ra lệnh" qua đúng 1 hàm. Sơ đồ máy trạng thái
đầy đủ: xem [app-lock.md](app-lock.md).

```c
typedef enum { APP_LOCK_STATE_LOCKED, APP_LOCK_STATE_UNLOCKED } app_lock_state_t;

void app_lock_init(void);
void app_lock_task(void);              // task scheduler, 100ms — chỉ kiểm hết 5s chưa
void app_lock_request_open(void);      // "command" duy nhất - mở (hoặc mở thêm) 5s
app_lock_state_t app_lock_get_state(void);
```

### app_led

Nhấp nháy LED "còn sống" (heartbeat), chu kỳ `APP_LED_BLINK_PERIOD_MS` = 1s — độc lập hoàn
toàn, không nhận input từ module nào khác (khác với LED xanh/đỏ mà UC-02/03/04/05 vẫn còn thiếu,
xem [Kết nối & Pinout](../hardware/connections.md#9-led-trang-thai)). `app_led_task()` chỉ
toggle mỗi lần được gọi — lịch gọi mỗi `APP_LED_BLINK_PERIOD_MS/2` (500ms) trong
`app_main.c`'s `g_pTaskTable`, không phải mỗi 1000ms, vì 1 lần toggle chỉ là nửa chu kỳ.

```c
#define APP_LED_BLINK_PERIOD_MS  (1000u)

void app_led_init(void);
void app_led_task(void);   // task scheduler, gọi mỗi APP_LED_BLINK_PERIOD_MS/2 (500ms)
```

### app_exit_button

Event handler cho `BSP_EXIT_BUTTON_EVENT_PRESSED` (`bsp_exit_button`, `event`) — gọi
`app_lock_request_open()` (mở khóa, UC-03), `app_system_notify_exit_pressed()` (chỉ để
`app_system` biết hiển thị "Exiting..." lên LCD nếu đang ở state `IDLE` - xem
[app-system.md](app-system.md)), và `app_log_record()` (UC-09). **Tự debounce 200ms** ở đây
(không phải trong ISR) — xem [app-lock.md](app-lock.md) để biết chi tiết + lý do đổi so với bản
trước (từng chủ động không debounce).

```c
void app_exit_button_init(void);
void app_exit_button_on_press(void);   // event handler, xem g_pEventTable trong app_main
```

### app_system

**Bộ điều phối trung tâm** - máy trạng thái đầy đủ (13 state: `IDLE`, `ADMIN_AUTH`,
`ADMIN_MENU`, `ADMIN_ADD_USER`, `ADMIN_ADD_USER_NAME`, `ADMIN_DEL_USER`, `ADMIN_CHANGE_PW1/2`,
`ADMIN_FACTORY_RESET_CONFIRM`, `ADMIN_MESSAGE`, `ACCESS_GRANTED`, `ACCESS_DENIED`, `LOCKED_OUT`,
`EXITING`) điều phối `app_rc522`/`app_keypad`/`app_persist`/`app_rtc` → `app_lock` + nội dung
LCD. Sơ đồ đầy đủ, bảng data-input/output, lý do thiết kế: xem
[app-system.md](app-system.md) - chi tiết quá nhiều để lặp lại ở đây.

```c
void app_system_init(void);
void app_system_task(void);   // task scheduler, 100ms

void app_system_notify_exit_pressed(void);   // gọi bởi app_exit_button, xem trên

typedef struct {
    char szLine1[APP_SYSTEM_DISPLAY_LINE_LEN];  // 17 = 16 ký tự LCD + NUL
    char szLine2[APP_SYSTEM_DISPLAY_LINE_LEN];
} app_system_display_t;

bool app_system_consume_display_if_updated(app_system_display_t *pOut, uint32_t *pui32LastSeenSeq);
```

Tóm tắt những gì state machine này **đã** làm thật (UC-02/03/04/05/06/07/08/09 — xem
[use-cases.md](../use-cases.md)): mở khóa khi quẹt thẻ khớp; hiển thị "Exiting..." khi nhấn nút
exit; tạm khoá sau 5 lần sai liên tiếp - **2 bộ đếm độc lập**, một cho quẹt thẻ sai
(`s_ui32FailedAttempts`), một cho nhập sai admin password (`s_ui32AdminFailedAttempts`), cùng
đích `LOCKED_OUT` (UC-05, cả 2 nhánh cuối cùng đã được xây); vào menu quản trị bằng master
password rồi thêm/xoá user, đổi master password, hoặc factory reset (xác nhận lại bằng master
password, UC-10) — cả 4 đều gọi trực tiếp API đã có của `app_persist`; ghi log qua
`app_log_record()` khi quẹt thẻ khớp/không khớp (UC-09, xem
[app-log.md](app-log.md)). UC-01 (PIN mở cửa) không có nhánh nào trong state machine này — đã
loại bỏ khỏi phạm vi, xem [use-cases.md](../use-cases.md#uc-01-mo-khoa-bang-pin-a-loai-bo-khoi-pham-vi).

### app_lcd

Chỉ là "con rối" vẽ lại đúng nội dung `app_system` quyết định — không có logic/state nào khác.

```c
void app_lcd_init(void);
void app_lcd_task(void);   // task scheduler, 100ms
```

`app_lcd_task()` gọi `app_system_consume_display_if_updated()`; nếu có nội dung mới, gọi
`bsp_lcd_set_line(0, ...)`/`bsp_lcd_set_line(1, ...)` cho 2 dòng. Không bao giờ vẽ lại khi nội
dung không đổi - việc dedupe nằm ở `app_system_set_display()` (so sánh `strcmp` trước khi tăng
số phiên bản), không phải ở đây.

### app_log

Ring buffer ghi log (UC-09) trong EEPROM, ngay sau vùng của `app_persist`
(`APP_PERSIST_EEPROM_SIZE_BYTES`, không phải số viết cứng). Không có scheduler task - chỉ ghi
theo sự kiện, gọi trực tiếp từ `app_system`/`app_exit_button`. Chi tiết đầy đủ (byte layout, sơ
đồ luồng ghi, lý do không có CRC): xem [app-log.md](app-log.md).

```c
typedef enum {
    APP_LOG_SOURCE_RFID_GRANTED,
    APP_LOG_SOURCE_RFID_DENIED,
    APP_LOG_SOURCE_EXIT
} app_log_source_t;

void app_log_init(void);
void app_log_record(app_log_source_t eSource, const uint8_t pui8Uid[RC522_UID_LEN]);  // uid: NULL cho EXIT
```

!!! note "Chưa xây — là việc tiếp theo"
    Chưa có cách đọc lại log (UART/LCD) — dữ liệu ghi đúng nhưng chưa có giao diện xem lại.
