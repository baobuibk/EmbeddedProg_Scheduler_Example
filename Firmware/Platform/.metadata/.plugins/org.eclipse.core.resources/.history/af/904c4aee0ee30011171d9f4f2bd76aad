/*
 * app_lcd.c
 */

#include <stdint.h>

#include "app_lcd.h"
#include "bsp_lcd.h"
#include "app_system.h"

static uint32_t s_ui32LastSeenDisplaySeq = 0U;

void app_lcd_init(void)
{
    bsp_lcd_init();
}

void app_lcd_task(void)
{
    app_system_display_t display;

    if (app_system_consume_display_if_updated(&display, &s_ui32LastSeenDisplaySeq)) {
        bsp_lcd_set_line(0U, display.szLine1);
        bsp_lcd_set_line(1U, display.szLine2);
    }
}
