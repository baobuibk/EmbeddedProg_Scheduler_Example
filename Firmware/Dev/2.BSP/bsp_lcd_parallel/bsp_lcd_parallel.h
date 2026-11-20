/*
 * bsp_lcd_parallel.h
 *
 * Board wiring for the directly-GPIO-connected 16x2 LCD (see
 * image/LCD.png): RS=PB9, RW=PB10, E=PB11, D4=PB12, D5=PB13, D6=PB14,
 * D7=PB15, all push-pull output. Provides the strong definitions of
 * lcd1602_parallel_rs_write()/_rw_write()/_en_write()/_data_write()
 * declared in Dev/3.Drivers/Devices/lcd1602_parallel/lcd1602_parallel.h -
 * nothing else in this header is needed by callers of that driver.
 *
 * Distinct from Dev/2.BSP/bsp_lcd/ (the PCF8574-over-I2C1 LCD wiring) -
 * image/LCD.png wires the LCD directly to GPIO instead, so the two BSPs
 * are alternatives for the same physical LCD1602 depending on which
 * board/schematic is actually populated, not meant to be used together.
 *
 * bsp_lcd_parallel_set_line() is the only drawing primitive - same shape
 * and same padding/truncation behavior as bsp_lcd.h's bsp_lcd_set_line(),
 * so app_lcd.c can swap between the two BSPs with no logic change.
 */

#pragma once

#include <stdint.h>

/* Configures RS/RW/E/D4-D7 as OUTPUT (push-pull), all idle low, then
 * runs the LCD's own power-on init sequence. Call once before the first
 * bsp_lcd_parallel_set_line(). */
void bsp_lcd_parallel_init(void);

/* ui8Row: 0 or 1. pszText: any length, NUL-terminated - clipped to 16
 * chars if longer, space-padded to 16 if shorter. */
void bsp_lcd_parallel_set_line(uint8_t ui8Row, const char *pszText);
