/*
 * dio.c
 *
 *  Created on: Aug 3, 2025
 *      Author: Admin
 */
#include "dio.h"
#include "definitions.h"

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "peripheral/pio/plib_pio.h"    // SYS function prototypes
#include "BSP_Board/board.h"
#include "BSP_Led/bsp_led.h"
#include "config/default/interrupts.h"

#define GPIO_MAX_PORTS 5  // A to E

#define PIO_PTR(x) ((pio_registers_t *)(x))

static pio_registers_t * const gpio_ports[GPIO_MAX_PORTS + 1] = {
    NULL,
    PIO_PTR(PIO_PORT_A),
    PIO_PTR(PIO_PORT_B),
    PIO_PTR(PIO_PORT_C),
    PIO_PTR(PIO_PORT_D),
    PIO_PTR(PIO_PORT_E),
};

static uint32_t const gpio_clock_en[GPIO_MAX_PORTS + 1] = {
    0,
    ID_PIOA,
    ID_PIOB,
    ID_PIOC,
    ID_PIOD,
    ID_PIOE
};

static void gpio_enable_clock(uint32_t ui32PortNum) {
    if (ui32PortNum == 0 || ui32PortNum > GPIO_MAX_PORTS) {
        return;
    }

    uint32_t periph_id = gpio_clock_en[ui32PortNum];

    if (periph_id < 32U) {
        PMC_REGS->PMC_PCER0 = (1UL << periph_id);
    } else {
        PMC_REGS->PMC_PCER1 = (1UL << (periph_id - 32U));
    }
}

/* NULL for port 0 or an out-of-range port/pin */
static pio_registers_t *do_port(const dio_t *me) {
    if ((me == NULL) || (me->port == 0U) || (me->port > GPIO_MAX_PORTS) || (me->pin >= 32U)) {
        return NULL;
    }
    return gpio_ports[me->port];
}

void dio_flow_address_pulse(dio_t *me) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    /* Cortex-M7 DWT lock access register. */
    *((volatile uint32_t *)0xE0001FB0UL) = 0xC5ACCE55UL;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    do_set(me);
    dio_mode(me, OUTPUT);
    uint32_t start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < (CPU_CLOCK_FREQUENCY / 1000000u) * 200u) {}
    dio_mode(me, INPUT);
    __set_PRIMASK(primask);
}

void do_set(dio_t *me) {
    pio_registers_t *port = do_port(me);
    if (port == NULL) return;
    uint32_t mask = (1u << me->pin);

    //    port->PIO_PER = mask;
    //    port->PIO_OER = mask;

    port->PIO_SODR = mask;

    me->bStatus = true;
}

void do_reset(dio_t *me) {
    pio_registers_t *port = do_port(me);
    if (port == NULL) return;
    uint32_t mask = (1u << me->pin);

    //    port->PIO_PER = mask;
    //    port->PIO_OER = mask;

    port->PIO_CODR = mask;

    me->bStatus = false;
}

void do_toggle(dio_t *me) {
    pio_registers_t *port = do_port(me);
    if (port == NULL) return;
    uint32_t mask = (1u << me->pin);

    //    port->PIO_PER = mask;
    //    port->PIO_OER = mask;

    if (((port->PIO_ODSR) & mask) != 0u) {
        port->PIO_CODR = mask;
        me->bStatus = false;
    } else {
        port->PIO_SODR = mask;
        me->bStatus = true;
    }
}

void dio_mode(dio_t *me, dio_mode_t mode) {
    pio_registers_t *port = do_port(me);
    if (port == NULL)
        return;
    uint32_t mask = (1u << me->pin);

    port->PIO_PER = mask;

    switch (mode) {
        case OUTPUT:
            port->PIO_PUDR = mask;
            port->PIO_PPDDR = mask;
            port->PIO_OER = mask;
            break;

        case INPUT:
            port->PIO_ODR = mask;
            port->PIO_PUDR = mask;
            port->PIO_PPDDR = mask;
            break;

        case INPUT_PULLUP:
            port->PIO_ODR = mask;
            port->PIO_PUER = mask;
            port->PIO_PPDDR = mask;
            break;

        case INPUT_PULLDOWN:
            port->PIO_ODR = mask;
            port->PIO_PUDR = mask;
            port->PIO_PPDER = mask;
            break;
    }
}

bool di_read(dio_t *me)
{
    pio_registers_t *port = do_port(me);
    if (port == NULL) return false;
    uint32_t mask = (1u << me->pin);

    bool bLevel = ((port->PIO_PDSR & mask) != 0u);
    me->bStatus = bLevel;

    return bLevel;
}
