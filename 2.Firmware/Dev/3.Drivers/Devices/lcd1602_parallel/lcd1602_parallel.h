/*
 * lcd1602_parallel.h
 *
 * Driver for an HD44780-compatible 16x2 LCD wired directly to GPIO in
 * 4-bit parallel mode (RS/RW/E + D4-D7) - see image/LCD.png (RS=PB9,
 * RW=PB10, E=PB11, D4-D7=PB12-PB15). Unlike lcd1602_i2c (the
 * PCF8574-backpack version), this driver has no bus handle and no
 * per-instance state at all - every pin is fixed board wiring, owned
 * entirely by the BSP through four weak-overridable hook functions (same
 * separation of concerns as keypad4x4.h's keypad_col_write/
 * keypad_row_read):
 *
 *   - lcd1602_parallel_rs_write(bLevel)   - drive RS (0=command, 1=data)
 *   - lcd1602_parallel_rw_write(bLevel)   - drive RW (always 0 here - see below)
 *   - lcd1602_parallel_en_write(bLevel)   - drive the EN strobe
 *   - lcd1602_parallel_data_write(nibble) - drive D4-D7 from the low
 *     nibble of the byte (bit0->D4 ... bit3->D7)
 *
 * The real, board-specific definitions live in
 * Dev/2.BSP/bsp_lcd_parallel/bsp_lcd_parallel.c. The weak defaults below
 * are no-ops, so anything linking against this driver without that BSP
 * still builds (same as every other weak-pin-function driver here).
 *
 * Write-only, like lcd1602_i2c: this driver always drives RW low and
 * never reads the busy flag back from D7, so every instruction's minimum
 * execution time is covered by a fixed delay instead (see the DWT-based
 * delay helper in lcd1602_parallel.c).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "define.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define LCD1602_PARALLEL_COLS   (16u)
#define LCD1602_PARALLEL_ROWS   (2u)

/******************************************************************************/
/*********************** Board-specific pin functions **************************/
/******************************************************************************/

/*
 * Weak defaults (see lcd1602_parallel.c) - a real board provides the
 * strong definitions in Dev/2.BSP/bsp_lcd_parallel/bsp_lcd_parallel.c.
 */
void lcd1602_parallel_rs_write(bool bLevel);
void lcd1602_parallel_rw_write(bool bLevel);
void lcd1602_parallel_en_write(bool bLevel);
void lcd1602_parallel_data_write(uint8_t ui8Nibble);

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

/* Runs the standard HD44780 power-on 4-bit init sequence (function set,
 * display on/cursor off/blink off, entry mode left-to-right). */
uint32_t lcd1602_parallel_init(void);

uint32_t lcd1602_parallel_clear(void);
uint32_t lcd1602_parallel_home(void);
/* ui8Col: 0-15, ui8Row: 0-1 */
uint32_t lcd1602_parallel_set_cursor(uint8_t ui8Col, uint8_t ui8Row);

uint32_t lcd1602_parallel_write_char(char c);
/* Stops at the first NUL - does not wrap or clip to the display width. */
uint32_t lcd1602_parallel_print(const char *pcStr);

uint32_t lcd1602_parallel_display_on_off(bool bOn);

/* Low-level: send a raw HD44780 command/data byte (RS=0/RS=1
 * respectively) - exposed for custom commands (e.g. CGRAM glyphs). */
uint32_t lcd1602_parallel_command(uint8_t ui8Cmd);
uint32_t lcd1602_parallel_data(uint8_t ui8Data);
