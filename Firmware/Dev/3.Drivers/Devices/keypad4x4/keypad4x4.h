/*
 * keypad4x4.h
 *
 * Driver for a 4x4 matrix keypad. Unlike most drivers under
 * Dev/3.Drivers/Devices/, this one does not take dio_t pins in its init
 * struct - it instead calls two board-specific functions, declared here
 * and given a weak (overridable) default definition in keypad4x4.c:
 *
 *   - keypad_col_write(ui8Col, bLevel): drive one column line
 *   - keypad_row_read(ui8Row): read one row line
 *
 * A real implementation lives in Dev/2.BSP/bsp_keypad.c (wired to this
 * board's actual pins - PA0-3 for the 4 columns, PA4-7 for the 4 rows -
 * see 1.Document/docs/hardware/connections.md) and overrides the weak
 * stub at link time. This lets the driver (and anything that scans a
 * keypad) build and link without a BSP present, and keeps the pin
 * assignment out of Dev/3.Drivers entirely - same separation of concerns
 * as i2c_io.h/spi_io.h/dio.h being platform-agnostic, just one layer
 * re-cut: here it's the *board*, not the *platform*, that's abstracted
 * away.
 *
 * Scan technique: drive one COL low at a time (others high), read all 4
 * ROWs (pulled up) - the first ROW read low identifies the pressed key
 * at (row, col) via keypad4x4_keymap. Only reports one key at a time (no
 * diodes assumed).
 *
 * keypad4x4_scan() is meant to be called periodically (e.g. from a
 * scheduler task - see Dev/4.MiddleWare/scheduler): it debounces
 * internally and reports each physical press exactly once, as a
 * one-shot event, not a level - the caller must call it repeatedly
 * regardless of whether a key was returned.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "define.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define KEYPAD4X4_ROWS      (4u)
#define KEYPAD4X4_COLS      (4u)

/* Consecutive matching scans required before a keypress is accepted -
 * tune to the caller's scan period (e.g. 3 ticks at a 20ms scheduler
 * period is ~60ms of debounce). */
#ifndef KEYPAD4X4_DEBOUNCE_COUNT
#define KEYPAD4X4_DEBOUNCE_COUNT   (3u)
#endif

#define KEYPAD4X4_NO_KEY     ('\0')

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

typedef struct
{
    char    cLastRawKey;
    uint8_t ui8DebounceCount;
    bool    bKeyReported;
} keypad4x4_t;

/******************************************************************************/
/*********************** Board-specific pin functions **************************/
/******************************************************************************/

/*
 * Weak defaults (see keypad4x4.c) - a real board provides the strong
 * definitions in Dev/2.BSP/bsp_keypad.c. ui8Col/ui8Row are 0-3, indexing
 * the same order as keypad4x4_keymap's rows/columns.
 */
void keypad_col_write(uint8_t ui8Col, bool bLevel);
bool keypad_row_read(uint8_t ui8Row);

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

uint32_t keypad4x4_init(keypad4x4_t *me);

/* One immediate scan, no debounce - returns the currently-pressed key's
 * character (see keypad4x4_keymap in keypad4x4.c), or KEYPAD4X4_NO_KEY. */
char keypad4x4_scan_raw(keypad4x4_t *me);

/* Debounced, one-shot: returns the pressed key's character exactly once
 * per physical press (KEYPAD4X4_NO_KEY otherwise, including while a key
 * is still held down after having already been reported). */
char keypad4x4_scan(keypad4x4_t *me);
