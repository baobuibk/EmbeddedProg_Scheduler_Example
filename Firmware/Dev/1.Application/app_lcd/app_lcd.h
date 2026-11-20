/*
 * app_lcd.h
 *
 * Dumb renderer - the only thing this module does is poll
 * app_system_consume_display_if_updated() and paint whatever it gets
 * onto the LCD via bsp_lcd. It owns no state of its own and makes no
 * decisions about *what* to show - that's entirely app_system's job
 * (see 1.Document/docs/firmware/app-system.md). This mirrors how
 * app_lock only executes open/close commands without deciding when to
 * issue them.
 */

#pragma once

void app_lcd_init(void);

/* Scheduler task - see app_main.c's g_pTaskTable. */
void app_lcd_task(void);
