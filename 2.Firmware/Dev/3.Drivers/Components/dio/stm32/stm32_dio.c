/*
 * stm32_dio.c
 *
 * STM32F103 backend for the do.h digital-IO abstraction (same role as
 * ../nxp/do.c and ../samv71/samv71_do.c for their platforms).
 *
 * me->port: 1=GPIOA, 2=GPIOB, 3=GPIOC (this part only exposes A/B/C).
 * me->pin:  0-15.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "do.h"
#include "stm32f1xx.h"

#define GPIO_MAX_PORTS  3U /* STM32F103C6 (LQFP48) only exposes GPIOA/B/C */

static GPIO_TypeDef *const gpio_ports[GPIO_MAX_PORTS + 1U] = {
    NULL,
    GPIOA,
    GPIOB,
    GPIOC
};

static const uint32_t gpio_clock_en[GPIO_MAX_PORTS + 1U] = {
    0U,
    RCC_APB2ENR_IOPAEN,
    RCC_APB2ENR_IOPBEN,
    RCC_APB2ENR_IOPCEN
};

/* NULL for port 0, an out-of-range port, or pin >= 16 */
static GPIO_TypeDef *do_port(const do_t *me)
{
    if ((me == NULL) || (me->port == 0U) || (me->port > GPIO_MAX_PORTS) || (me->pin >= 16U)) {
        return NULL;
    }

    return gpio_ports[me->port];
}

static void gpio_enable_clock(uint32_t port_num)
{
    if ((port_num == 0U) || (port_num > GPIO_MAX_PORTS)) {
        return;
    }

    RCC->APB2ENR |= gpio_clock_en[port_num];
}

void do_set(do_t *me)
{
    GPIO_TypeDef *port = do_port(me);

    if (port == NULL) {
        return;
    }

    port->BSRR = (1UL << me->pin);
    me->bStatus = true;
}

void do_reset(do_t *me)
{
    GPIO_TypeDef *port = do_port(me);

    if (port == NULL) {
        return;
    }

    port->BSRR = (1UL << (me->pin + 16U));
    me->bStatus = false;
}

void do_toggle(do_t *me)
{
    GPIO_TypeDef *port = do_port(me);

    if (port == NULL) {
        return;
    }

    uint32_t mask = (1UL << me->pin);

    if ((port->ODR & mask) != 0U) {
        port->BSRR = mask << 16U;
        me->bStatus = false;
    } else {
        port->BSRR = mask;
        me->bStatus = true;
    }
}

void dio_mode(do_t *me, dio_mode_t mode)
{
    GPIO_TypeDef *port = do_port(me);

    if (port == NULL) {
        return;
    }

    gpio_enable_clock(me->port);

    uint32_t pin = me->pin;
    volatile uint32_t *cr = (pin < 8U) ? &port->CRL : &port->CRH;
    uint32_t shift = (pin % 8U) * 4U;
    uint32_t mode_bits;
    uint32_t cnf_bits;

    switch (mode) {
        case OUTPUT:
            mode_bits = 0x3U; /* 50MHz */
            cnf_bits = 0x0U;  /* general-purpose push-pull */
            break;

        case INPUT_PULLUP:
        case INPUT_PULLDOWN:
            mode_bits = 0x0U;
            cnf_bits = 0x2U;  /* input, pull-up/down selected via ODR below */
            break;

        case INPUT:
        default:
            mode_bits = 0x0U;
            cnf_bits = 0x1U;  /* floating input */
            break;
    }

    *cr &= ~(0xFUL << shift);
    *cr |= ((cnf_bits << 2U) | mode_bits) << shift;

    if (mode == INPUT_PULLUP) {
        port->BSRR = (1UL << pin);
    } else if (mode == INPUT_PULLDOWN) {
        port->BSRR = (1UL << (pin + 16U));
    }
}

bool di_read(do_t *me)
{
    GPIO_TypeDef *port = do_port(me);

    if (port == NULL) {
        return false;
    }

    bool level = ((port->IDR & (1UL << me->pin)) != 0U);
    me->bStatus = level;

    return level;
}

/* SLF3x address-select pulse: 200us is far too short for the scheduler's
 * 1ms tick (os_tick_get()), so this uses the Cortex-M3 DWT cycle counter
 * directly instead - same technique as ../samv71/samv71_do.c, minus the
 * DWT lock-access-register unlock that backend needs (Cortex-M7/M33
 * specific; M3's DWT isn't locked by default). */
void dio_flow_address_pulse(do_t *me)
{
    extern uint32_t SystemCoreClock;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    dio_mode(me, OUTPUT);
    do_set(me);

    uint32_t start = DWT->CYCCNT;
    uint32_t cycles_200us = (SystemCoreClock / 1000000U) * 200U;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles_200us) {
        /* busy-wait */
    }

    dio_mode(me, INPUT);

    if (primask == 0U) {
        __enable_irq();
    }
}
