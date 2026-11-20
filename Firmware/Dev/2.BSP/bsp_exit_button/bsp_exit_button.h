/*
 * bsp_exit_button.h
 *
 * Board wiring for the exit button: PA11, input pull-up, active-low
 * (see 1.Document/docs/hardware/connections.md #3). Configures PA11 as
 * EXTI11 falling-edge and posts BSP_EXIT_BUTTON_EVENT_PRESSED
 * (Dev/4.MiddleWare/event) from EXTI15_10_IRQHandler on every press.
 *
 * Unlike RC522's IRQ pin (tried and deliberately reverted - see
 * Dev/2.BSP/bsp_rc522.h), this IS a genuine fit for the event module: a
 * button press is a one-shot, truly asynchronous input with nothing to
 * periodically re-ask (compare RC522, where a passive card can't signal
 * on its own and the MCU has to keep sending REQA regardless of IRQ).
 * EXTI here really does replace polling an input pin every scheduler
 * tick, instead of just duplicating a wait the driver already does
 * synchronously.
 *
 * No software debounce here: contact bounce can fire this ISR/event
 * more than once in a row, but app_exit_button's handler just calls
 * app_lock_request_open() again - and app_lock already treats "another
 * open request while already open" as "restart the 5s window", which
 * is harmless to call redundantly. Debounce would only matter if this
 * pin fed something order/count-sensitive, which it doesn't.
 */

#pragma once

/* Index into app_main.c's g_pEventTable[] - see Dev/4.MiddleWare/event
 * and bsp_rc522.h's BSP_RC522_EVENT_CARD_PRESENT for the same pattern
 * (named constant, not a bare literal, read on both the posting and
 * the table-placement side). */
#define BSP_EXIT_BUTTON_EVENT_PRESSED (0U)

void bsp_exit_button_init(void);
