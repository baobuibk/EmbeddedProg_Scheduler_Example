/*
 * lcd1602_parallel.c
 */

#include <stddef.h>
#include <stdbool.h>

#include "lcd1602_parallel.h"
#include "define.h"
#include "stm32f1xx.h"

/* HD44780 instructions (same bit layout as lcd1602_i2c.c) */
#define LCD_CMD_CLEAR_DISPLAY       (0x01u)
#define LCD_CMD_RETURN_HOME         (0x02u)
#define LCD_CMD_ENTRY_MODE_SET      (0x04u)
#define LCD_CMD_DISPLAY_CONTROL     (0x08u)
#define LCD_CMD_FUNCTION_SET        (0x20u)
#define LCD_CMD_SET_DDRAM_ADDR      (0x80u)

#define LCD_ENTRY_LEFT_TO_RIGHT     (0x02u)
#define LCD_DISPLAY_ON              (0x04u)
#define LCD_FUNCTION_4BIT_2LINE     (0x08u)

/* Minimum instruction execution time per the HD44780 datasheet - most
 * instructions need >37us, CLEAR_DISPLAY/RETURN_HOME need >1.52ms. Used
 * as fixed post-command delays since this driver never reads the busy
 * flag back (see the header comment). Generous versus the datasheet
 * minimums - this only costs a few ms total per call, and init only
 * runs once at boot. */
#define LCD_DELAY_SHORT_US    (50u)
#define LCD_DELAY_CLEAR_US    (2000u)

/* Row 0/1 DDRAM base addresses, standard HD44780 16x2 layout. */
static const uint8_t lcd_row_offset[LCD1602_PARALLEL_ROWS] = { 0x00U, 0x40U };

/*
 * Weak stubs: let anything that links against this driver build without
 * a BSP present - same pattern as keypad4x4.c's keypad_col_write/
 * keypad_row_read stubs. See lcd1602_parallel.h / bsp_lcd_parallel.c.
 */
__attribute__((weak)) void lcd1602_parallel_rs_write(bool bLevel)
{
    (void)bLevel;
}

__attribute__((weak)) void lcd1602_parallel_rw_write(bool bLevel)
{
    (void)bLevel;
}

__attribute__((weak)) void lcd1602_parallel_en_write(bool bLevel)
{
    (void)bLevel;
}

__attribute__((weak)) void lcd1602_parallel_data_write(uint8_t ui8Nibble)
{
    (void)ui8Nibble;
}

/*
 * DWT cycle-counter busy-wait - same technique
 * Dev/3.Drivers/Components/dio/stm32/stm32_dio.c's
 * dio_flow_address_pulse() already uses for its own sub-ms SLF3x pulse,
 * generalized here to an arbitrary microsecond count: HD44780 timing (EN
 * pulse width, per-instruction execution time) is sub-millisecond, below
 * os_tick_get()'s 1ms resolution, and this driver has no busy-flag/other
 * status bit to poll instead (write-only, RW always low - see the header
 * comment) - the DWT cycle counter is the only observable "time has
 * passed" signal available. Unlike dio_flow_address_pulse, this does NOT
 * disable interrupts: an occasional ISR only makes the wait a little
 * longer, never shorter, and the longest delay here (the ~40ms power-on
 * wait in lcd1602_parallel_init()) would otherwise stall the scheduler
 * tick for far too long.
 */
static void lcd1602_parallel_delay_us(uint32_t us)
{
    extern uint32_t SystemCoreClock;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = (SystemCoreClock / 1000000U) * us;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
        /* busy-wait */
    }
}

/* One EN strobe: RS/RW/data must already be set up on the bus. Pulse
 * widths below are far more generous than the HD44780's ~450ns minimum -
 * cheap insurance against a marginal setup/hold violation. */
static void lcd1602_parallel_strobe(void)
{
    lcd1602_parallel_delay_us(1U); /* address/data setup time */
    lcd1602_parallel_en_write(true);
    lcd1602_parallel_delay_us(1U); /* EN pulse width */
    lcd1602_parallel_en_write(false);
    lcd1602_parallel_delay_us(1U); /* EN cycle time / data hold */
}

static void lcd1602_parallel_write_nibble(uint8_t ui8Nibble, bool bRs)
{
    lcd1602_parallel_rs_write(bRs);
    lcd1602_parallel_rw_write(false); /* always write - see header comment */
    lcd1602_parallel_data_write(ui8Nibble & 0x0FU);
    lcd1602_parallel_strobe();
}

static void lcd1602_parallel_send_byte(uint8_t ui8Byte, bool bRs, uint32_t ui32DelayUs)
{
    lcd1602_parallel_write_nibble((uint8_t)(ui8Byte >> 4U), bRs);
    lcd1602_parallel_write_nibble(ui8Byte, bRs);
    lcd1602_parallel_delay_us(ui32DelayUs);
}

uint32_t lcd1602_parallel_command(uint8_t ui8Cmd)
{
    bool bClear = (ui8Cmd == LCD_CMD_CLEAR_DISPLAY) || (ui8Cmd == LCD_CMD_RETURN_HOME);

    lcd1602_parallel_send_byte(ui8Cmd, false, bClear ? LCD_DELAY_CLEAR_US : LCD_DELAY_SHORT_US);

    return ERROR_OK;
}

uint32_t lcd1602_parallel_data(uint8_t ui8Data)
{
    lcd1602_parallel_send_byte(ui8Data, true, LCD_DELAY_SHORT_US);

    return ERROR_OK;
}

uint32_t lcd1602_parallel_init(void)
{
    /* Power-on init: force 4-bit mode starting from an unknown state (the
     * LCD may already be in 8-bit mode from a previous boot) - the
     * standard three-times-0x3-then-0x2 nibble sequence every HD44780
     * datasheet/app-note specifies, each wait generous versus the
     * minimums (>15ms, >4.1ms, >100us, >100us). */
    lcd1602_parallel_delay_us(40000U);

    lcd1602_parallel_write_nibble(0x03U, false);
    lcd1602_parallel_delay_us(5000U);

    lcd1602_parallel_write_nibble(0x03U, false);
    lcd1602_parallel_delay_us(200U);

    lcd1602_parallel_write_nibble(0x03U, false);
    lcd1602_parallel_delay_us(200U);

    lcd1602_parallel_write_nibble(0x02U, false); /* switch to 4-bit */
    lcd1602_parallel_delay_us(200U);

    lcd1602_parallel_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_4BIT_2LINE);
    lcd1602_parallel_command(LCD_CMD_DISPLAY_CONTROL); /* display off while configuring */
    lcd1602_parallel_clear();
    lcd1602_parallel_command(LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT_TO_RIGHT);
    lcd1602_parallel_display_on_off(true);

    return ERROR_OK;
}

uint32_t lcd1602_parallel_clear(void)
{
    return lcd1602_parallel_command(LCD_CMD_CLEAR_DISPLAY);
}

uint32_t lcd1602_parallel_home(void)
{
    return lcd1602_parallel_command(LCD_CMD_RETURN_HOME);
}

uint32_t lcd1602_parallel_set_cursor(uint8_t ui8Col, uint8_t ui8Row)
{
    if ((ui8Row >= LCD1602_PARALLEL_ROWS) || (ui8Col >= LCD1602_PARALLEL_COLS)) {
        return ERROR_INVALID_PARAM;
    }

    return lcd1602_parallel_command(LCD_CMD_SET_DDRAM_ADDR | (lcd_row_offset[ui8Row] + ui8Col));
}

uint32_t lcd1602_parallel_write_char(char c)
{
    return lcd1602_parallel_data((uint8_t)c);
}

uint32_t lcd1602_parallel_print(const char *pcStr)
{
    if (pcStr == NULL) {
        return ERROR_INVALID_PARAM;
    }

    while (*pcStr != '\0') {
        lcd1602_parallel_write_char(*pcStr);
        pcStr++;
    }

    return ERROR_OK;
}

uint32_t lcd1602_parallel_display_on_off(bool bOn)
{
    return lcd1602_parallel_command(LCD_CMD_DISPLAY_CONTROL | (bOn ? LCD_DISPLAY_ON : 0U));
}
