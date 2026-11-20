/*
 * app_system.h
 *
 * The central orchestrator: the only module that reads app_rc522,
 * app_keypad and app_persist together and decides both "what state is
 * the system in" and "what the LCD should show". app_lock and app_lcd
 * are both "dumb" relative to this module - app_lock only executes
 * open/close commands, app_lcd only paints whatever text this module
 * last decided on. See 1.Document/docs/firmware/app-system.md for the
 * full state diagram this implements.
 *
 * UC-01 (unlock by PIN) is explicitly OUT OF SCOPE - see
 * 1.Document/docs/use-cases.md's UC-01 entry for why (a PIN cannot
 * reliably distinguish two different people the way a UID can, unless
 * the admin enforces uniqueness, and even then two people can still
 * share one on purpose). The keypad is only ever used here to enter the
 * ADMIN password (UC-06/07/08), never to unlock the door.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define APP_SYSTEM_DISPLAY_LINE_LEN  (17u)  /* 16 visible chars + NUL */

typedef struct
{
    char szLine1[APP_SYSTEM_DISPLAY_LINE_LEN];
    char szLine2[APP_SYSTEM_DISPLAY_LINE_LEN];
}
app_system_display_t;

void app_system_init(void);

/* Scheduler task - see app_main.c's g_pTaskTable. */
void app_system_task(void);

/* Called by app_exit_button_on_press() IN ADDITION TO (not instead of)
 * its existing app_lock_request_open() call - app_lock still doesn't
 * know app_system exists. Only changes anything if currently IDLE (see
 * app-system.md); the lock opens regardless of this call's effect. */
void app_system_notify_exit_pressed(void);

/* *pui32LastSeenSeq: caller-owned, zero-initialize before the first
 * call - same pattern as app_rtc_consume_if_updated()/
 * app_rc522_consume_if_updated(). Only app_lcd is expected to call
 * this. */
bool app_system_consume_display_if_updated(app_system_display_t *pOut, uint32_t *pui32LastSeenSeq);
