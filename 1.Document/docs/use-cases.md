# Use Cases

Đề xuất các kịch bản sử dụng (use case) cho hệ thống Access Control.

!!! warning "Phạm vi"
    Tạm thời **bỏ qua** các use case liên quan đến:

    - Giao tiếp Wi-Fi qua module ESP8266 (đồng bộ server, gửi log từ xa, OTA...).
    - Tương tác lệnh qua UART debug console.

    Hai nhóm use case này sẽ được bổ sung ở trang riêng khi phạm vi hệ thống mở rộng
    ra kết nối mạng và công cụ vận hành/debug.

!!! info "Cột 'Trạng thái' dưới đây phản ánh firmware THẬT, không phải thiết kế"
    Trang này mô tả use case ở mức **ý định thiết kế** — luồng chi tiết của một UC có thể vẫn mô
    tả bước LED/LCD/log dù module đó chưa tồn tại. Cột Trạng thái trong bảng tổng hợp và dòng
    "Trạng thái firmware" trong mỗi UC nói rõ phần nào đã chạy thật (có trong `g_pTaskTable`/
    `g_pEventTable` của `app_main.c`, xem [Task & Data Flow](firmware/tasks-dataflow.md)) và phần
    nào vẫn chỉ là API/thiết kế chưa có ai gọi.

## Actor

| Actor | Vai trò |
|---|---|
| **Người dùng (User)** | Người cần mở khóa để ra/vào — xác thực bằng **thẻ RFID** (đã quyết định bỏ PIN cho user thường, xem UC-01). |
| **Quản trị viên (Admin)** | Người quản lý hệ thống — thêm/xoá người dùng, đổi PIN quản trị, xem lịch sử. |
| **Hệ thống (System)** | STM32F103 cùng các khối phần cứng — tự động thực hiện auto re-lock, log, cảnh báo. |

## Bảng tổng hợp Use Case

| ID | Use case | Actor | Thành phần liên quan | Trạng thái firmware |
|---|---|---|---|---|
| UC-01 | ~~Mở khóa bằng PIN~~ | ~~User~~ | ~~Keypad, LCD, EEPROM, Lock, LED~~ | 🚫 Đã loại bỏ khỏi phạm vi |
| UC-02 | Mở khóa bằng thẻ RFID | User | RC522, LCD, EEPROM, Lock, LED | ✅ Đã chạy (thiếu LED) |
| UC-03 | Mở khóa từ bên trong | User | Exit Button, Lock, LED | ✅ Đã chạy (thiếu LED) |
| UC-04 | Tự động đóng khóa sau khi mở | System | Lock, LED | ✅ Đã chạy (thiếu LED) |
| UC-05 | Cảnh báo xác thực sai nhiều lần | System | Keypad/RC522, LCD, LED | ✅ Đã chạy cho cả RFID và admin password (2 bộ đếm riêng, thiếu LED) |
| UC-06 | Quản trị viên thêm người dùng mới (thẻ + tên) | Admin | Keypad, RC522, LCD, EEPROM | ✅ Đã chạy (tên gõ tay bằng phím A-D) |
| UC-07 | Quản trị viên xoá người dùng | Admin | Keypad, LCD, EEPROM | ✅ Đã chạy |
| UC-08 | Đổi PIN quản trị (master PIN) | Admin | Keypad, LCD, EEPROM | ✅ Đã chạy |
| UC-09 | Ghi log lịch sử ra/vào | System | EEPROM | ✅ Đã chạy (ring buffer 2000 record) |
| UC-10 | Khôi phục cấu hình mặc định (factory reset) | Admin | Keypad, LCD, EEPROM | ✅ Đã chạy (trigger qua menu, xác nhận lại bằng admin password) |

## Chi tiết các use case chính

### UC-01 — ~~Mở khóa bằng PIN~~ (đã loại bỏ khỏi phạm vi)

!!! danger "Quyết định: bỏ hẳn PIN cho user thường, chỉ dùng thẻ RFID"
    Trong lúc thiết kế [bảng PIN người dùng](firmware/app-system.md) cho UC-01, nảy ra câu hỏi:
    "mở khóa bằng PIN thì làm sao phân biệt 2 người khác nhau?" — câu trả lời là **không đảm
    bảo được tự nhiên** như UID (UID gắn vật lý với 1 thẻ, nhà sản xuất đảm bảo duy nhất; PIN chỉ
    là 1 chuỗi số, 2 người có thể dùng chung PIN nếu không ai chặn, và hệ thống không thể phân
    biệt họ khi đó). Vì lý do này, **quyết định bỏ hẳn** việc mở khóa bằng PIN cho user thường —
    hệ thống chỉ xác thực user qua **thẻ RFID** (UC-02). Giữ lại mục này (gạch ngang, không xoá)
    để ghi nhận quyết định và lý do, phòng trường hợp sau này cân nhắc lại.

    **Không ảnh hưởng đến bàn phím nói chung**: `app_keypad`/phím số vẫn còn dùng — cho việc
    **nhập master password để vào menu quản trị** (UC-06/07/08, xem `ADMIN_AUTH` ở
    [app-system.md](firmware/app-system.md)). Đó là xác thực 1-mật-khẩu-duy-nhất-của-admin, khác
    hẳn bản chất với "PIN riêng cho mỗi user thường" vừa bị loại bỏ — admin chỉ có 1 người, không
    có vấn đề phân biệt nhiều người dùng chung 1 bí mật.

### UC-02 — Mở khóa bằng thẻ RFID

- **Mô tả:** Người dùng đưa thẻ/tag RFID vào vùng đọc của module RC522, hệ thống so
  khớp UID thẻ với danh sách UID hợp lệ đã lưu.
- **Luồng chính:**
    1. System gửi lệnh REQA định kỳ (mỗi 200ms — `app_rc522_task`, xem
       [Module đã viết](firmware/modules.md#app_rc522)) để hỏi "có thẻ trong vùng từ
       trường không". Chân **IRQ** (PB5) vẫn đi dây trên board nhưng firmware hiện
       **không dùng ngắt EXTI** cho nó — bộ lập lịch cooperative và REQA đã block
       đồng bộ chờ SPI xong nên một ngắt thêm không tận dụng được gì (xem lý do chi
       tiết ở [Kết nối & Pinout](hardware/connections.md#6-module-rfid-rc522-spi)).
    2. Nếu REQA có trả lời, đọc đầy đủ UID (anticollision + SELECT qua SPI) ngay
       trong cùng lần gọi đó, rồi đưa thẻ vào trạng thái HALT để không bị trigger
       lại liên tục khi thẻ còn nằm trong vùng từ trường.
    3. `app_system_task()` (100ms) phát hiện UID mới qua `app_rc522_consume_if_updated()`,
       so khớp qua `app_persist_user_find_by_uid()`.
    4. Khớp → `app_lock_request_open()` (mở 5s, xem UC-04), LCD hiện "Access Granted" + tên user
       2s, ghi log `APP_LOG_SOURCE_RFID_GRANTED` + UID (UC-09). LED xanh **chưa được gọi**.
    5. Không khớp → LCD hiện "Access Denied"/"Unknown Card" 2s, ghi log
       `APP_LOG_SOURCE_RFID_DENIED` + UID (UC-09), tăng bộ đếm sai liên tiếp (UC-05) — vượt
       ngưỡng 5 lần thì vào `LOCKED_OUT` luôn (xem UC-05). LED đỏ **chưa được gọi**.
- **Trạng thái firmware:** ✅ Toàn bộ đã chạy thật (`app_rc522` + `app_persist` + `app_system` +
  `app_lock` + `app_lcd` + `app_log`, xem [app-system.md](firmware/app-system.md) và
  [app-log.md](firmware/app-log.md)). Thiếu: LED.

### UC-03 — Mở khóa từ bên trong (Exit Button)

- **Mô tả:** Người dùng nhấn nút exit để mở khóa đi ra, không cần xác thực bằng thẻ.
- **Luồng chính:**
    1. Nhấn nút exit (PA11) kéo chân xuống thấp → ngắt **EXTI11** nổ ngay (`bsp_exit_button`'s
       `EXTI15_10_IRQHandler`), post sự kiện `BSP_EXIT_BUTTON_EVENT_PRESSED` (chỉ set 1 cờ, an
       toàn trong ngắt).
    2. `event_run()` (main loop, ngay sau `scheduler_run()`) thấy cờ, gọi
       `app_exit_button_on_press()` → `app_lock_request_open()` → mở khóa ngay (xem UC-04).
    3. LCD hiện "Exiting..." 2s (nếu đang `IDLE` lúc nhấn - xem `app_system_notify_exit_pressed()`
       ở [app-system.md](firmware/app-system.md)). Ghi log `APP_LOG_SOURCE_EXIT` (UC-09) **không
       điều kiện** - xảy ra mỗi lần nhấn nút, bất kể `app_system` có đang `IDLE` để hiện LCD hay
       không (khác với phần hiển thị). LED xanh **chưa được gọi**.
- **Lưu ý:** Không tính vào bộ đếm nhập sai (UC-05) vì không phải thao tác xác thực — đúng với
  firmware thật (`app_exit_button` không chạm gì tới bộ đếm, vì nút exit không phải một lần "xác
  thực sai/đúng").
- **Không có debounce phần mềm** cho chân này: rung tiếp điểm có thể làm ngắt nổ vài lần liên
  tiếp, nhưng mỗi lần chỉ gọi lại `app_lock_request_open()` — vô hại vì hàm đó chỉ "mở/gia hạn
  thêm 5s", gọi lặp lại không gây sai lệch (nhưng sẽ ghi log 1 record mỗi lần nổ ngắt - một rung
  mạnh có thể tạo vài record liên tiếp cùng giây, chấp nhận được, log không khử trùng). Xem
  [app-lock.md](firmware/app-lock.md).
- **Trạng thái firmware:** ✅ Đã chạy thật, kể cả đường ngắt EXTI (đây là consumer thật đầu tiên
  của [`event`](firmware/modules.md#event)), phản hồi LCD, và ghi log. Thiếu: LED.

### UC-04 — Tự động đóng khóa sau khi mở

- **Mô tả:** Sau khi khóa được mở hợp lệ (UC-02/UC-03), hệ thống tự động
  đóng khóa lại sau một khoảng thời gian cấu hình (hiện 5 giây, `APP_LOCK_OPEN_DURATION_MS`),
  không chờ thao tác thêm từ người dùng.
- **Luồng chính:**
    1. Bất kỳ nguồn nào gọi `app_lock_request_open()` → nếu đang đóng, mở khóa
       (`bsp_lock_open()`); **luôn** ghi lại mốc thời gian hiện tại (`os_tick_get()`), bất kể
       trước đó đang đóng hay đã mở — đây cũng chính là cơ chế "có thẻ/nút nhấn khác trong 5s thì
       mở thêm 5s" mà không cần logic riêng.
    2. `app_lock_task()` (scheduler, 100ms) kiểm tra: đã trôi qua `>= 5000ms` từ mốc thời gian đó
       chưa — nếu rồi, `bsp_lock_close()`, quay về trạng thái `LOCKED`.
- **Trạng thái firmware:** ✅ Đã chạy thật — xem máy trạng thái đầy đủ (`stateDiagram-v2`) ở
  [app-lock.md](firmware/app-lock.md). `app_lock` tự đóng khóa độc lập, không đồng bộ với LCD -
  `app_system` của nó cũng tự quay về `IDLE` (hiện giờ + "Quet the de mo") sau 2s hiển thị thông
  báo riêng, 2 timer không liên quan nhau (xem [app-system.md](firmware/app-system.md)). Thiếu:
  LED trở về trạng thái idle (chưa có driver LED).

### UC-05 — Cảnh báo xác thực sai nhiều lần

- **Mô tả:** Nếu quét thẻ không hợp lệ **hoặc** nhập sai master password liên tiếp quá số lần
  cho phép, hệ thống tạm khóa việc xử lý tương ứng trong một khoảng thời gian để chống dò
  UID/mật khẩu (brute-force). 2 nguồn này dùng **2 bộ đếm độc lập**, không trộn chung — không
  phải phần còn lại của PIN-mở-cửa đã bị loại bỏ ở UC-01.
- **Luồng chính (nhánh RFID):**
    1. Mỗi lần quét thẻ không khớp (`IDLE`, xem UC-02 bước 5) → `s_ui32FailedAttempts++` trong
       `app_system`.
    2. Đạt `APP_SYSTEM_MAX_FAILED_ATTEMPTS` (= 5) → vào state `LOCKED_OUT` ngay (không qua
       `ACCESS_DENIED` nữa lần này).
    3. Quẹt thẻ khớp (`ACCESS_GRANTED`) reset `s_ui32FailedAttempts` về 0 ngay, không cần đợi
       hết lockout.
- **Luồng chính (nhánh master password):**
    1. Mỗi lần `ADMIN_AUTH`'s `#` không khớp `app_persist_admin_password_check()` →
       `s_ui32AdminFailedAttempts++` (biến **riêng**, không chung với nhánh RFID).
    2. Đạt `APP_SYSTEM_MAX_FAILED_ATTEMPTS` (= 5, cùng ngưỡng với nhánh RFID) → vào `LOCKED_OUT`.
    3. Khớp password đúng (vào được `ADMIN_MENU`) reset `s_ui32AdminFailedAttempts` về 0 ngay.
- **Khi đã vào `LOCKED_OUT`** (từ nguồn nào cũng vậy): LCD hiện "Locked, wait:" + số giây còn
  lại, đếm ngược mỗi 100ms. `app_system_tick_locked_out()` là nhánh duy nhất chạy mỗi tick - UID/
  phím đều bị `app_system_task()` **bỏ qua hoàn toàn** cho tới khi hết giờ. Hết
  `APP_SYSTEM_LOCKOUT_DURATION_MS` (= 30s) → về `IDLE`, **cả 2** bộ đếm reset về 0 (không chỉ bộ
  đếm đã gây ra lockout lần đó).
- **Trạng thái firmware:** ✅ Đã chạy thật cho **cả 2 nhánh** (xem
  [app-system.md](firmware/app-system.md)). Thiếu LED đỏ nhấp nháy (chưa có driver LED nào trong
  cả dự án).

### UC-06 — Quản trị viên thêm người dùng mới

- **Mô tả:** Admin vào chế độ quản trị (nhập master password), quét thẻ mới để lưu lại.
- **Luồng chính:**
    1. Admin bấm 1 phím số (bất kỳ) ở `IDLE` → vào `ADMIN_AUTH`, nhập master password, `#` xác
       nhận → khớp thì vào `ADMIN_MENU` (LCD hiện "1Add 2Del 3Pw" / "4Exit").
    2. Bấm `1` → vào `ADMIN_ADD_USER`, LCD hiện "Scan new card" / "then type name".
    3. Quẹt thẻ mới → UID được lưu tạm, chuyển sang `ADMIN_ADD_USER_NAME`, LCD hiện
       "Name(A-D),# ok" + tên đang gõ.
    4. Admin gõ tên bằng các phím chữ `A`-`D` (bàn phím 4x4 chỉ có 4 phím chữ, không có `E`/`F`),
       `#` xác nhận → `app_persist_user_add(uid, tên)`.
    5. LCD xác nhận "User added:" + tên (hoặc "Card already registered"/"Add failed: table full"
       nếu trùng/đầy) trong 2s, rồi quay lại `ADMIN_MENU`.
- **Trạng thái firmware:** ✅ Đã chạy thật — xem [app-system.md](firmware/app-system.md) và
  [app-persist.md](firmware/app-persist.md) (`app_persist_user_add()` làm toàn bộ việc kiểm
  trùng/ghi). Tên gõ tay bằng 4 chữ `A`-`D` (không phải A-F như đề xuất ban đầu - bàn phím 4x4
  không có phím `E`/`F`), không còn tự sinh như bản trước.

### UC-07 — Quản trị viên xoá người dùng

- **Mô tả:** Admin chọn một thẻ đã lưu để xoá khỏi danh sách hợp lệ.
- **Luồng chính:** Từ `ADMIN_MENU`, bấm `2` → vào `ADMIN_DEL_USER`, LCD hiện tên user đầu tiên
  (`app_persist_user_get_by_index(0, ...)`). `*` = xem user kế tiếp (vòng lại từ đầu khi hết
  danh sách), `#` = xoá user đang hiện (`app_persist_user_remove()`), phím khác = huỷ về
  `ADMIN_MENU`.
- **Trạng thái firmware:** ✅ Đã chạy thật — xem [app-system.md](firmware/app-system.md).

### UC-08 — Đổi PIN quản trị (master PIN)

- **Mô tả:** Admin đổi mã PIN dùng để vào menu quản trị, PIN mới được lưu lại
  (vùng cấu hình riêng, phân biệt với danh sách UID người dùng).
- **Luồng chính:** Từ `ADMIN_MENU`, bấm `3` → nhập password mới, `#` → nhập lại lần 2 để xác
  nhận, `#` → nếu 2 lần khớp, `app_persist_admin_password_set()` ghi mới, LCD "Password changed";
  nếu không khớp, LCD "Mismatch, try again" và **không ghi gì** (giữ password cũ).
- **Trạng thái firmware:** ✅ Đã chạy thật — xem [app-system.md](firmware/app-system.md) và
  [app-persist.md](firmware/app-persist.md).

### UC-09 — Ghi log lịch sử ra/vào

- **Mô tả:** Mỗi sự kiện mở khóa thành công/thất bại được ghi vào vùng log trong
  EEPROM 24C256 (dạng ring buffer do dung lượng hạn chế), gồm nguồn xác thực
  (RFID/Exit), kết quả, và thời điểm.
- **Lưu ý:** Thiết kế phần cứng hiện tại đã có module **RTC DS1307** (I2C, xem
  [Hardware](hardware/index.md)) nên log ghi mốc thời gian thực (ngày/giờ) thay vì chỉ số thứ
  tự/tick hệ thống — miễn DS1307 đã được set giờ đúng lần đầu (qua menu quản trị hoặc đồng bộ
  qua Wi-Fi, mục sau nằm ngoài phạm vi trang này); nếu RTC chưa từng đọc được, record vẫn được
  ghi nhưng với mốc thời gian toàn số 0 (không bỏ qua record chỉ vì thiếu giờ).
- **Trạng thái firmware:** ✅ Đã chạy thật — module mới `app_log` (xem
  [app-log.md](firmware/app-log.md)), ring buffer 2000 record (13 byte/record: giờ/ngày/tháng/
  năm + nguồn + UID) bắt đầu ngay sau vùng của `app_persist`
  (`APP_PERSIST_EEPROM_SIZE_BYTES` — hằng số tính toán, không phải số 2117 viết cứng, để không
  lệch nếu layout `app_persist` đổi lần nữa). Gọi từ `app_system` (RFID khớp/không khớp) và
  `app_exit_button` (nút exit) — không có scheduler task riêng, chỉ ghi theo sự kiện. **Không
  có công cụ đọc lại log** (ví dụ qua UART/LCD) — dữ liệu chỉ nằm trong EEPROM, chưa có cách xem
  lại từ bên ngoài phạm vi trang hiện tại (UART debug console nằm ngoài phạm vi, xem đầu trang).

### UC-10 — Khôi phục cấu hình mặc định (Factory reset)

- **Mô tả:** Cho phép admin đưa hệ thống về trạng thái mặc định (xoá toàn bộ UID người dùng, đặt
  lại master password mặc định) khi cần tái triển khai.
- **Luồng chính:**
    1. Từ `ADMIN_MENU`, bấm `5` → vào `ADMIN_FACTORY_RESET_CONFIRM`, LCD hiện "Confirm w/pass:".
    2. Admin **nhập lại** master password (dù vừa xác thực để vào menu) — một lớp xác nhận riêng
       cho thao tác không thể hoàn tác này, `#` xác nhận.
    3. Khớp → `app_persist_factory_reset()` (xoá bảng UID, đặt lại password mặc định `"1234"`),
       LCD "Factory reset"/"done" 2s, rồi về **`IDLE`** (không về lại `ADMIN_MENU` — password cũ
       vừa dùng để vào menu không còn hợp lệ nữa sau reset).
    4. Không khớp → LCD "Wrong password"/"cancelled" 2s rồi về `ADMIN_MENU`, tăng
       `s_ui32AdminFailedAttempts` (UC-05, **cùng bộ đếm** với `ADMIN_AUTH`) — vượt ngưỡng cũng
       vào `LOCKED_OUT` như mọi lần nhập sai password khác.
- **Lưu ý:** Thiết kế này **không** còn phục vụ được trường hợp "quên mật khẩu quản trị" (một lý
  do ban đầu từng nêu) — bước xác nhận chính là nhập lại password đó, nên nếu quên thì không qua
  được bước xác nhận. Đây là đánh đổi có chủ đích theo đúng yêu cầu "xác nhận bằng password
  admin": an toàn hơn (không ai reset được nếu không biết password hiện tại, kể cả chính admin
  đang đăng nhập), nhưng không phải cơ chế khôi phục khi quên mật khẩu — nếu cần đường khôi phục
  đó, cần thiết kế riêng (ví dụ tổ hợp phím vật lý lúc cấp nguồn, nằm ngoài phạm vi hiện tại).
- **Trạng thái firmware:** ✅ Đã chạy thật — xem [app-system.md](firmware/app-system.md) và
  [app-persist.md](firmware/app-persist.md).
  `ADMIN_MENU` (xem [app-system.md](firmware/app-system.md)) hiện chỉ có 4 lựa chọn
  (Add/Del/Đổi password/Exit) — chưa có lựa chọn thứ 5 cho factory reset.
