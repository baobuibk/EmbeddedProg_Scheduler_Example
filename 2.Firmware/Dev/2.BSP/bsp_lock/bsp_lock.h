/*
 * bsp_lock.h
 *
 * Board wiring for the electric lock's control line: PA8
 * (LOCK_CTRL -> relay/MOSFET driver + opto-isolation -> 12V lock, see
 * 1.Document/docs/hardware/connections.md #3). Pure GPIO, no device
 * driver layer needed (same reasoning as bsp_keypad's pins - there's no
 * chip/protocol here, just one output pin).
 *
 * Polarity assumed fail-secure: PA8 LOW (idle/reset) = lock de-energized
 * = physically LOCKED; PA8 HIGH = relay energized = unlocked. This is
 * the common default for an access-control strike/solenoid (locked
 * when unpowered, e.g. on a power cut) - if the actual relay/driver
 * board wired up is active-low or fail-safe instead, swap
 * do_set()/do_reset() in bsp_lock_open()/close() below, nothing above
 * this file needs to change.
 */

#pragma once

void bsp_lock_init(void);

void bsp_lock_open(void);
void bsp_lock_close(void);
