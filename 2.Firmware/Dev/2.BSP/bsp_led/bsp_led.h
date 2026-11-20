/*
 * bsp_led.h
 *
 * Board wiring for the status LED's control line: PC13 (LED_STATUS ->
 * current-limiting resistor -> LED, see
 * 1.Document/docs/hardware/connections.md #9). Pure GPIO, no device
 * driver layer needed - same reasoning as bsp_lock's pin (there's no
 * chip/protocol here, just one output pin).
 *
 * Polarity assumed active-HIGH: PC13 HIGH = LED on, LOW (idle/reset) =
 * LED off. If the actual board wires the LED the other way around
 * (e.g. a Blue-Pill-style onboard LED, which is active-low), swap
 * do_set()/do_reset() in bsp_led_on()/off() below - nothing above this
 * file needs to change.
 */

#pragma once

void bsp_led_init(void);

void bsp_led_on(void);
void bsp_led_off(void);
void bsp_led_toggle(void);
