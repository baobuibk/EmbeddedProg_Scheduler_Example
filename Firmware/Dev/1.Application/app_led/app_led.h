/*
 * app_led.h
 *
 * Status LED blink, period APP_LED_BLINK_PERIOD_MS (1s: 500ms on,
 * 500ms off) - purely cosmetic/heartbeat, no input from any other
 * module. app_led_task() just toggles on every call, so the scheduler
 * entry (app_main.c's g_pTaskTable) must run it every
 * APP_LED_BLINK_PERIOD_MS/2, not every APP_LED_BLINK_PERIOD_MS - one
 * toggle is half a cycle (on->off or off->on), two toggles make one
 * full 1s period.
 */

#pragma once

#define APP_LED_BLINK_PERIOD_MS  (1000u)

void app_led_init(void);

/* Scheduler task - see app_main.c's g_pTaskTable. Call every
 * APP_LED_BLINK_PERIOD_MS/2 ms. */
void app_led_task(void);
