/*
 * bsp_keypad.h
 *
 * Board wiring for the 4x4 keypad: columns on PA0-3, rows on PA4-7 (see
 * 1.Document/docs/hardware/connections.md). Provides the strong
 * definitions of keypad_col_write()/keypad_row_read() declared in
 * Dev/3.Drivers/Devices/keypad4x4/keypad4x4.h - nothing else in this
 * header is needed by callers of the keypad4x4 driver itself.
 */

#pragma once

/* Configures PA0-3 as OUTPUT (idle high) and PA4-7 as INPUT_PULLUP.
 * Call once before the first keypad4x4_scan()/keypad4x4_init(). */
void bsp_keypad_init(void);
