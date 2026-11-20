/*
 * bsp_lock.c
 */

#include "bsp_lock.h"
#include "dio.h"

#define BSP_LOCK_GPIO_PORT_A  (1u)  /* matches dio.h's port numbering (1=GPIOA) */

static dio_t s_lockPin = { .port = BSP_LOCK_GPIO_PORT_A, .pin = 8U };

void bsp_lock_init(void)
{
    dio_mode(&s_lockPin, OUTPUT);
    do_reset(&s_lockPin); /* idle = locked - see bsp_lock.h's polarity note */
}

void bsp_lock_open(void)
{
    do_set(&s_lockPin);
}

void bsp_lock_close(void)
{
    do_reset(&s_lockPin);
}
