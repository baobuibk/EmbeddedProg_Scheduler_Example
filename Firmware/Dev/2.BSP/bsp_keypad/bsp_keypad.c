/*
 * bsp_keypad.c
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "bsp_keypad.h"
#include "keypad4x4.h"
#include "dio.h"

#define BSP_KEYPAD_GPIO_PORT_A   (1u)   /* matches dio.h's port numbering (1=GPIOA) */

/* COL0-3 = PA0-3 (driven), ROW0-3 = PA4-7 (read) - see
 * 1.Document/docs/hardware/connections.md. Index order matches
 * keypad4x4_keymap in keypad4x4.c. */
static dio_t bsp_keypad_col_pins[KEYPAD4X4_COLS] = {
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 0U },
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 1U },
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 2U },
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 3U },
};

static dio_t bsp_keypad_row_pins[KEYPAD4X4_ROWS] = {
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 4U },
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 5U },
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 6U },
    { .port = BSP_KEYPAD_GPIO_PORT_A, .pin = 7U },
};

void bsp_keypad_init(void)
{
    for (uint32_t i = 0U; i < KEYPAD4X4_COLS; i++) {
        dio_mode(&bsp_keypad_col_pins[i], OUTPUT);
        do_set(&bsp_keypad_col_pins[i]); /* idle high */
    }

    for (uint32_t i = 0U; i < KEYPAD4X4_ROWS; i++) {
        dio_mode(&bsp_keypad_row_pins[i], INPUT_PULLUP);
    }
}

void keypad_col_write(uint8_t ui8Col, bool bLevel)
{
    if (ui8Col >= KEYPAD4X4_COLS) {
        return;
    }

    if (bLevel) {
        do_set(&bsp_keypad_col_pins[ui8Col]);
    } else {
        do_reset(&bsp_keypad_col_pins[ui8Col]);
    }
}

bool keypad_row_read(uint8_t ui8Row)
{
    if (ui8Row >= KEYPAD4X4_ROWS) {
        return true; /* out-of-range reads as "not pressed" */
    }

    return di_read(&bsp_keypad_row_pins[ui8Row]);
}
