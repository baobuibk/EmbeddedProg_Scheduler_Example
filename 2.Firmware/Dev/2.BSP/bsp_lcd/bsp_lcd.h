/*
 * bsp_lcd.h
 *
 * Wires the lcd1602_i2c driver to this board's shared I2C1 bus
 * (bsp_i2c1), addr 0x27 (PCF8574) - see
 * 1.Document/docs/hardware/connections.md #2. Same shape as
 * bsp_rtc/bsp_eeprom: thin wrapper, no caching, real I2C transaction on
 * every call.
 *
 * bsp_lcd_set_line() is the only drawing primitive - it always pads/
 * truncates to exactly LCD1602_COLS characters before printing, so the
 * caller never has to worry about leftover characters from a previous,
 * longer line still showing on screen. Callers (app_lcd) just pass
 * whatever text they have; the display-painting mechanics belong here,
 * not in Application.
 */

#pragma once

#include <stdint.h>

void bsp_lcd_init(void);

/* ui8Row: 0 or 1. pszText: any length, NUL-terminated - clipped to 16
 * chars if longer, space-padded to 16 if shorter. */
void bsp_lcd_set_line(uint8_t ui8Row, const char *pszText);
