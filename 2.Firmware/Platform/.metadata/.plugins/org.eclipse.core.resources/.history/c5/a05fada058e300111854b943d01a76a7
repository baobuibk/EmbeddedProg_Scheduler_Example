/*
 * bsp_lcd.c
 */

#include "bsp_lcd.h"
#include "lcd1602_i2c.h"
#include "bsp_i2c1.h"

static lcd1602_t s_lcd;

void bsp_lcd_init(void)
{
    bsp_i2c1_init(); /* idempotent - see bsp_i2c1.h */

    lcd1602_init_t init = { .pI2cBus = bsp_i2c1_get(), .ui16DevAddr = LCD1602_I2C_ADDR_PCF8574 };

    lcd1602_init(&s_lcd, &init);
//    lcd1602_backlight(&s_lcd, true);
}

void bsp_lcd_set_line(uint8_t ui8Row, const char *pszText)
{
    char pcPadded[LCD1602_COLS + 1U];
    uint8_t ui8Index = 0U;

    for (; (ui8Index < LCD1602_COLS) && (pszText[ui8Index] != '\0'); ui8Index++) {
        pcPadded[ui8Index] = pszText[ui8Index];
    }

    for (; ui8Index < LCD1602_COLS; ui8Index++) {
        pcPadded[ui8Index] = ' ';
    }

    pcPadded[LCD1602_COLS] = '\0';

    lcd1602_set_cursor(&s_lcd, 0U, ui8Row);
    lcd1602_print(&s_lcd, pcPadded);
}
