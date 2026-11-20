/*
 * app_led.c
 */

#include "app_led.h"
#include "bsp_led.h"

void app_led_init(void)
{
    bsp_led_init();
}

void app_led_task(void)
{
    bsp_led_toggle();
}
