/*
 * keypad4x4.h
 *
 * Driver for a 4x4 matrix keypad (see
 * 1.Document/docs/hardware/connections.md #1 - 4 ROW outputs + 4 COL
 * inputs, no diodes assumed), built on the platform-agnostic do.h
 * GPIO abstraction.
 *
 * Scan technique: drive one ROW low at a time (others left high),
 * read all 4 COLs (pulled up), the first COL read low identifies the
 * pressed key at (row, col) via keypad4x4_keymap - standard approach
 * for a keypad with no row/column diodes, which only ever reports one
 * key at a time.
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

#include "do.h"
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
    do_t *pRows[KEYPAD4X4_ROWS];   /* ROW1..ROW4 - driven, active-low scan */
    do_t *pCols[KEYPAD4X4_COLS];   /* COL1..COL4 - read, pulled up */
} keypad4x4_init_t;

typedef struct
{
    do_t   *pRows[KEYPAD4X4_ROWS];
    do_t   *pCols[KEYPAD4X4_COLS];
    char    cLastRawKey;
    uint8_t ui8DebounceCount;
    bool    bKeyReported;
} keypad4x4_t;

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

/* Configures pRows as OUTPUT (idle high) and pCols as INPUT_PULLUP. */
uint32_t keypad4x4_init(keypad4x4_t *me, const keypad4x4_init_t *init);

/* One immediate scan, no debounce - returns the currently-pressed key's
 * character (see keypad4x4_keymap in keypad4x4.c), or KEYPAD4X4_NO_KEY. */
char keypad4x4_scan_raw(keypad4x4_t *me);

/* Debounced, one-shot: returns the pressed key's character exactly once
 * per physical press (KEYPAD4X4_NO_KEY otherwise, including while a key
 * is still held down after having already been reported). */
char keypad4x4_scan(keypad4x4_t *me);
