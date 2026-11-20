/*
 * keypad4x4.c
 */

#include <stddef.h>
#include <stdbool.h>

#include "keypad4x4.h"
#include "do.h"
#include "define.h"

/* Standard 4x4 keypad legend - row/col order matches the pRows[]/pCols[]
 * arrays passed to keypad4x4_init() (ROW1..4, COL1..4). */
static const char keypad4x4_keymap[KEYPAD4X4_ROWS][KEYPAD4X4_COLS] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' },
};

uint32_t keypad4x4_init(keypad4x4_t *me, const keypad4x4_init_t *init)
{
    if (me == NULL || init == NULL) {
        return ERROR_INVALID_PARAM;
    }

    for (uint32_t i = 0U; i < KEYPAD4X4_ROWS; i++) {
        if (init->pRows[i] == NULL) {
            return ERROR_INVALID_PARAM;
        }

        me->pRows[i] = init->pRows[i];
        dio_mode(me->pRows[i], OUTPUT);
        do_set(me->pRows[i]); /* idle high - only the row being scanned goes low */
    }

    for (uint32_t i = 0U; i < KEYPAD4X4_COLS; i++) {
        if (init->pCols[i] == NULL) {
            return ERROR_INVALID_PARAM;
        }

        me->pCols[i] = init->pCols[i];
        dio_mode(me->pCols[i], INPUT_PULLUP);
    }

    me->cLastRawKey = KEYPAD4X4_NO_KEY;
    me->ui8DebounceCount = 0U;
    me->bKeyReported = false;

    return ERROR_OK;
}

char keypad4x4_scan_raw(keypad4x4_t *me)
{
    if (me == NULL) {
        return KEYPAD4X4_NO_KEY;
    }

    char key = KEYPAD4X4_NO_KEY;

    for (uint32_t row = 0U; row < KEYPAD4X4_ROWS; row++) {
        do_reset(me->pRows[row]);

        for (uint32_t col = 0U; col < KEYPAD4X4_COLS; col++) {
            if (!di_read(me->pCols[col])) {
                key = keypad4x4_keymap[row][col];
            }
        }

        do_set(me->pRows[row]);

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
