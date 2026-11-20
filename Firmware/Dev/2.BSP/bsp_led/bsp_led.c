/*
 * bsp_led.c
 */

#include "bsp_led.h"
#include "dio.h"

#define BSP_LED_GPIO_PORT_C  (3u)  /* matches dio.h's port numbering (3=GPIOC) */

static dio_t s_ledPin = { .port = BSP_LED_GPIO_PORT_C, .pin = 13U };

void bsp_led_init(void)
{
    dio_mode(&s_ledPin, OUTPUT);
    do_reset(&s_ledPin); /* idle = off - see bsp_led.h's polarity note */
}

void bsp_led_on(void)
{
    do_set(&s_ledPin);
}

void bsp_led_off(void)
{
    do_reset(&s_ledPin);
}

void bsp_led_toggle(void)
{
    do_toggle(&s_ledPin);
}
