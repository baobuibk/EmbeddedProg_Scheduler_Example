/*
 * keypad4x4.c
 */

#include <stddef.h>
#include <stdbool.h>

#include "keypad4x4.h"
#include "define.h"

/* Standard 4x4 keypad legend - row/col order matches the index (0-3)
 * passed to keypad_row_read()/keypad_col_write(). */
static const char keypad4x4_keymap[KEYPAD4X4_ROWS][KEYPAD4X4_COLS] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' },
};

/*
 * Weak stubs: let anything that links against this driver build without
 * a BSP present. bLevel=true/returning true both mean "idle" (column
 * released / row not pulled low), so a no-BSP build's keypad4x4_scan()
 * always reports KEYPAD4X4_NO_KEY instead of a false keypress - see the
 * header comment for how Dev/2.BSP/bsp_keypad.c overrides these.
 */
__attribute__((weak)) void keypad_col_write(uint8_t ui8Col, bool bLevel)
{
    (void)ui8Col;
    (void)bLevel;
}

__attribute__((weak)) bool keypad_row_read(uint8_t ui8Row)
{
    (void)ui8Row;
    return true;
}

uint32_t keypad4x4_init(keypad4x4_t *me)
{
    if (me == NULL) {
        return ERROR_INVALID_PARAM;
    }

    me->cLastRawKey = KEYPAD4X4_NO_KEY;
    me->ui8DebounceCount = 0U;
    me->bKeyReported = false;

    for (uint32_t col = 0U; col < KEYPAD4X4_COLS; col++) {
        keypad_col_write((uint8_t)col, true); /* idle high - only the column being scanned goes low */
    }

    return ERROR_OK;
}

char keypad4x4_scan_raw(keypad4x4_t *me)
{
    if (me == NULL) {
        return KEYPAD4X4_NO_KEY;
    }

    char key = KEYPAD4X4_NO_KEY;

    for (uint32_t col = 0U; col < KEYPAD4X4_COLS; col++) {
        keypad_col_write((uint8_t)col, false);

        for (uint32_t row = 0U; row < KEYPAD4X4_ROWS; row++) {
            if (!keypad_row_read((uint8_t)row)) {
                key = keypad4x4_keymap[row][col];
            }
        }

        keypad_col_write((uint8_t)col, true);

        if (key != KEYPAD4X4_NO_KEY) {
            break;
        }
    }

    return key;
}

char keypad4x4_scan(keypad4x4_t *me)
{
    if (me == NULL) {
        return KEYPAD4X4_NO_KEY;
    }

    char raw = keypad4x4_scan_raw(me);

    if (raw == me->cLastRawKey) {
        if (me->ui8DebounceCount < KEYPAD4X4_DEBOUNCE_COUNT) {
            me->ui8DebounceCount++;
        }
    } else {
        me->cLastRawKey = raw;
        me->ui8DebounceCount = 0U;
    }

    if (raw == KEYPAD4X4_NO_KEY) {
        me->bKeyReported = false;
        return KEYPAD4X4_NO_KEY;
    }

    if ((me->ui8DebounceCount >= KEYPAD4X4_DEBOUNCE_COUNT) && !me->bKeyReported) {
        me->bKeyReported = true;
        return raw;
    }

    return KEYPAD4X4_NO_KEY;
}
