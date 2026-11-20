/*
 * event.h
 *
 * Event-driven dispatch, separate from and complementary to
 * Dev/4.MiddleWare/scheduler (which only ever runs tasks by elapsed
 * time - see its own header). Modeled on the event mechanism in the
 * reference project's sys.c (SYS_PostEvent()/SYS_HandleEvents()), but
 * generalized: that file hard-coded one fixed enum and a dispatch table
 * local to one function; here, the event table is a client-declared
 * global (g_pEventTable[]/g_ui32EventNumEvents), the same ownership
 * pattern scheduler.h already uses for g_pTaskTable[] - so anyone
 * already familiar with wiring up g_pTaskTable in app_main.c wires up
 * events the same way.
 *
 * Typical use: an ISR (or any code, including a weak callback override
 * - see Dev/3.Drivers/Devices/keypad4x4's pattern for why a lower layer
 * should call a named callback instead of reaching into a higher
 * layer's table) calls event_post(ui32EventId). The event's ID is just
 * its index into g_pEventTable[] - a stable, named constant the poster
 * and the table both reference (an enum/#define in whichever header
 * makes sense for that event), never a raw number threaded by hand.
 * event_run() must be called every pass of the main loop (see
 * app_main.c), the same way scheduler_run() must be - unlike
 * scheduler_run(), there is no periodic timing involved: event_run()
 * just checks every entry's pending flag and calls its handler if set.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 * Nguyên mẫu hàm xử lý 1 sự kiện - cùng dạng với tSchedulerFunction của
 * scheduler, để 1 hàm có thể vừa là task định kỳ vừa là event handler
 * nếu cần, không cần viết 2 bản khác chữ ký.
 */
typedef void (*tEventHandler)(void);

typedef struct
{
    //
    //! Hàm được gọi khi sự kiện này được post và tiêu thụ. NULL = bỏ qua
    //! (post vẫn set bPending nhưng event_run() sẽ không gọi gì).
    //
    tEventHandler pfnHandler;

    //
    //! Cờ "đang chờ xử lý" - set bởi event_post() (an toàn gọi từ ISR),
    //! xoá bởi event_run() ngay TRƯỚC khi gọi pfnHandler (nếu một post
    //! mới xảy ra đúng lúc handler đang chạy, sự kiện đó không bị mất,
    //! chỉ dời sang lần event_run() kế tiếp). Phải là volatile vì được
    //! ghi từ ngữ cảnh ngắt, đọc từ ngữ cảnh main loop.
    //
    volatile bool bPending;
}
tEventEntry;

/*
 * Bảng toàn cục này phải được điền bởi client (giống g_pTaskTable của
 * scheduler) - mỗi phần tử là 1 sự kiện, chỉ số của nó trong bảng
 * chính là "event ID" dùng cho event_post().
 */
extern tEventEntry g_pEventTable[];

/*
 * Biến toàn cục này phải được xuất bởi client. Nó chứa số lượng mục
 * trong mảng g_pEventTable.
 */
extern uint32_t g_ui32EventNumEvents;

/*
 * Đánh dấu sự kiện ui32EventId đang chờ xử lý, để event_run() gọi
 * handler của nó trong lần tiếp theo. An toàn gọi từ ISR - chỉ set 1
 * cờ volatile, không gọi handler trực tiếp từ ngữ cảnh ngắt. Không làm
 * gì nếu ui32EventId >= g_ui32EventNumEvents.
 */
void event_post(uint32_t ui32EventId);

/*
 * Duyệt toàn bộ g_pEventTable, gọi pfnHandler của mọi sự kiện đang
 * bPending (và xoá cờ trước khi gọi - xem tEventEntry.bPending ở trên).
 * Phải được gọi đều đặn từ main loop (xem app_main.c), không có
 * khái niệm "chu kỳ" ở đây - khác scheduler_run().
 */
void event_run(void);
