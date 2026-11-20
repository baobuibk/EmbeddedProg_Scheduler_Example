/*
 * app_keypad.h
 *
 * Application-level keypad task: wraps bsp_keypad_init() +
 * Dev/3.Drivers/Devices/keypad4x4 into a single scheduler task
 * (app_keypad_task - matches tSchedulerFunction's void(void) signature,
 * see Dev/4.MiddleWare/scheduler/scheduler.h) and holds its own result.
 *
 * Single-key mode only for now: each key press is reported as itself
 * (e.g. '2' reads as '2') - no Nokia-style multi-tap A-z letter cycling
 * yet. The result lives here rather than in a separate data module,
 * since there is currently only this one input source; revisit that if
 * a second input source (e.g. RC522) ever needs to feed the same
 * consumer.
 */

#pragma once

#include <stdbool.h>

void app_keypad_init(void);

/* Scheduler task - call periodically (e.g. every 20ms, matching
 * KEYPAD4X4_DEBOUNCE_COUNT's assumed scan period). Scans the keypad and,
 * on a newly-debounced key press, latches it as the pending key (see
 * app_keypad_get_key()). */
void app_keypad_task(void);

/* Retrieves and clears the most recently pressed key.
 * Returns true and writes *pcKey if a key was pending, false (leaving
 * *pcKey untouched) otherwise. Only the latest unread key is kept - a
 * second key pressed before the first is read overwrites it. */
bool app_keypad_get_key(char *pcKey);
