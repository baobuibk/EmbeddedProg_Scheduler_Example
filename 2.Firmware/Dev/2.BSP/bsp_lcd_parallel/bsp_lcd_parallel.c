/*
 * bsp_lcd_parallel.c
 *
 * Talks to GPIOB directly (RCC/CRH/BSRR, same no-HAL direct-register
 * style as Dev/3.Drivers/Components/dio/stm32/stm32_dio.c and
 * .../SPI/stm32/stm32_spi_io.c's own GPIO setup) instead of going
 * through the generic dio_t/do_set/do_reset/dio_mode abstraction used
 * elsewhere (bsp_keypad.c, bsp_lock.c, ...). All seven LCD pins
 * (RS=PB9, RW=PB10, EN=PB11, D4-D7=PB12-PB15) sit on the SAME port and
 * in fact the same CRH register (pins 8-15), and the four data pins are
 * contiguous bits (12-15) - so pin config is one read-modify-write
 * instead of seven dio_mode() calls, and a full data nibble is one BSRR
 * write instead of four separate dio_t set/reset calls. The nibble write
 * runs twice per byte sent to the LCD (once per nibble), so this isn't
 * just cosmetic - it's the hot path of this whole driver.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "bsp_lcd_parallel.h"
#include "lcd1602_parallel.h"
#include "stm32f1xx.h"

/* What's actually on the physical LCD right now, per row - kept in sync
 * with every bsp_lcd_parallel_set_line() call so it can diff against the
 * next one and only repaint the cells that actually changed (see that
 * function below). Initialized to all spaces in bsp_lcd_parallel_init(),
 * matching what lcd1602_parallel_init()'s own clear-display command
 * really leaves in the LCD's DDRAM. */
static char s_lastPainted[LCD1602_PARALLEL_ROWS][LCD1602_PARALLEL_COLS];

#define LCD_PIN_RS   (9U)
#define LCD_PIN_RW   (10U)
#define LCD_PIN_EN   (11U)
#define LCD_PIN_D4   (12U)   /* first data pin - D5/D6/D7 follow at D4+1/+2/+3 */

#define LCD_DATA_PINS_MASK  ((1UL << LCD_PIN_D4) | (1UL << (LCD_PIN_D4 + 1U)) | \
                              (1UL << (LCD_PIN_D4 + 2U)) | (1UL << (LCD_PIN_D4 + 3U)))
#define LCD_ALL_PINS_MASK   ((1UL << LCD_PIN_RS) | (1UL << LCD_PIN_RW) | (1UL << LCD_PIN_EN) | \
                              LCD_DATA_PINS_MASK)

void bsp_lcd_parallel_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* PB9-PB15 all live in CRH (pins 8-15, 4 bits/pin) - clear then set
     * each one's CNF/MODE nibble to "general-purpose push-pull output,
     * 50MHz" (CNF=00, MODE=11) in a single combined read-modify-write
     * covering all 7 pins, instead of 7 separate register updates. */
    uint32_t crh = GPIOB->CRH;

    for (uint32_t pin = LCD_PIN_RS; pin <= (LCD_PIN_D4 + 3U); pin++) {
        uint32_t shift = (pin - 8U) * 4U;
        crh &= ~(0xFUL << shift);
        crh |= (0x3UL << shift);
    }

    GPIOB->CRH = crh;
    GPIOB->BSRR = LCD_ALL_PINS_MASK << 16U; /* all idle low */

    lcd1602_parallel_init(); /* its own clear-display command writes spaces to every DDRAM cell */
    memset(s_lastPainted, ' ', sizeof(s_lastPainted));
}

/* Only repaints the cells that actually changed since the last call, not
 * the whole 16 characters - HD44780's DDRAM address auto-increments after
 * every write, so a run of consecutive changed cells only needs one
 * set_cursor; a fresh set_cursor is only re-issued after skipping over an
 * unchanged cell. Worst case (every cell different) costs the same as the
 * old whole-line repaint; the common case (e.g. a clock only changing 1-2
 * digits a second) is far cheaper. */
void bsp_lcd_parallel_set_line(uint8_t ui8Row, const char *pszText)
{
    if (ui8Row >= LCD1602_PARALLEL_ROWS) {
        return;
    }

    char pcPadded[LCD1602_PARALLEL_COLS];
    uint8_t ui8Index = 0U;

    for (; (ui8Index < LCD1602_PARALLEL_COLS) && (pszText[ui8Index] != '\0'); ui8Index++) {
        pcPadded[ui8Index] = pszText[ui8Index];
    }

    for (; ui8Index < LCD1602_PARALLEL_COLS; ui8Index++) {
        pcPadded[ui8Index] = ' ';
    }

    char *pLast = s_lastPainted[ui8Row];
    bool bCursorValid = false;

    for (uint8_t ui8Col = 0U; ui8Col < LCD1602_PARALLEL_COLS; ui8Col++) {
        if (pcPadded[ui8Col] == pLast[ui8Col]) {
            bCursorValid = false; /* next change (if any) is after a gap - needs a fresh set_cursor */
            continue;
        }

        if (!bCursorValid) {
            lcd1602_parallel_set_cursor(ui8Col, ui8Row);
            bCursorValid = true;
        }

        lcd1602_parallel_write_char(pcPadded[ui8Col]);
        pLast[ui8Col] = pcPadded[ui8Col];
    }
}

void lcd1602_parallel_rs_write(bool bLevel)
{
    GPIOB->BSRR = bLevel ? (1UL << LCD_PIN_RS) : (1UL << (LCD_PIN_RS + 16U));
}

void lcd1602_parallel_rw_write(bool bLevel)
{
    GPIOB->BSRR = bLevel ? (1UL << LCD_PIN_RW) : (1UL << (LCD_PIN_RW + 16U));
}

void lcd1602_parallel_en_write(bool bLevel)
{
    GPIOB->BSRR = bLevel ? (1UL << LCD_PIN_EN) : (1UL << (LCD_PIN_EN + 16U));
}

/* D4-D7 = PB12-PB15, contiguous - the whole nibble (bit0->D4 ... bit3->D7)
 * is driven with one atomic BSRR write: bits to set in the low half, bits
 * to reset in the high half (see stm32_dio.c's do_set/do_reset - same
 * BSRR mechanism, just combined here into a single write instead of up
 * to 4). */
void lcd1602_parallel_data_write(uint8_t ui8Nibble)
{
    uint32_t set_bits = ((uint32_t)(ui8Nibble & 0x0FU)) << LCD_PIN_D4;
    uint32_t reset_bits = (~set_bits) & LCD_DATA_PINS_MASK;

    GPIOB->BSRR = set_bits | (reset_bits << 16U);
}
