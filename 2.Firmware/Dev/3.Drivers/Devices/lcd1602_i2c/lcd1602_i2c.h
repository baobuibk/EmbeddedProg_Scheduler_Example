/*
 * lcd1602_i2c.h
 *
 * Driver for an HD44780 16x2 LCD behind a PCF8574(A) I2C backpack (see
 * 1.Document/docs/hardware/connections.md #2 - address 0x27/0x3F,
 * shared I2C1 bus with the EEPROM and RTC), built on the
 * platform-agnostic i2c_io.h bus abstraction.
 *
 * PCF8574 backpack pin mapping (the near-universal wiring for these
 * modules):
 *   P0=RS, P1=RW (always driven low - this driver never reads back from
 *   the LCD), P2=EN, P3=Backlight, P4-P7=D4-D7 (4-bit HD44780 mode).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "i2c_io.h"
#include "define.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define LCD1602_I2C_ADDR_PCF8574   (0x27u)
#define LCD1602_I2C_ADDR_PCF8574A  (0x3Fu)

#define LCD1602_COLS               (16u)
#define LCD1602_ROWS               (2u)

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

typedef struct
{
    i2c_io_t *pI2cBus;
    uint16_t  ui16DevAddr;  /* LCD1602_I2C_ADDR_PCF8574 or _PCF8574A */
} lcd1602_init_t;

typedef struct
{
    i2c_io_t *pI2cBus;
    uint16_t  ui16DevAddr;
    bool      bBacklightOn;
} lcd1602_t;

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

/* Runs the standard HD44780 power-on 4-bit init sequence (function set,
 * display on/cursor off/blink off, entry mode left-to-right). */
uint32_t lcd1602_init(lcd1602_t *me, const lcd1602_init_t *init);

uint32_t lcd1602_clear(lcd1602_t *me);
uint32_t lcd1602_home(lcd1602_t *me);
/* ui8Col: 0-15, ui8Row: 0-1 */
uint32_t lcd1602_set_cursor(lcd1602_t *me, uint8_t ui8Col, uint8_t ui8Row);

uint32_t lcd1602_write_char(lcd1602_t *me, char c);
/* Stops at the first NUL - does not wrap or clip to the display width. */
uint32_t lcd1602_print(lcd1602_t *me, const char *pcStr);

uint32_t lcd1602_backlight(lcd1602_t *me, bool bOn);
uint32_t lcd1602_display_on_off(lcd1602_t *me, bool bOn);

/* Low-level: send a raw HD44780 command/data byte (RS=0/RS=1
 * respectively) - exposed for custom commands (e.g. CGRAM glyphs). */
uint32_t lcd1602_command(lcd1602_t *me, uint8_t ui8Cmd);
uint32_t lcd1602_data(lcd1602_t *me, uint8_t ui8Data);
