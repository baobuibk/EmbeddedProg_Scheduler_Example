/*
 * app_lock.h
 *
 * Owns the lock's state machine (2 states, LOCKED/UNLOCKED - see the
 * state diagram in 1.Document/docs/firmware/app-lock.md) and the only
 * command this app accepts: app_lock_request_open(). This is the
 * decoupling point the whole design hinges on - app_lock has ZERO
 * knowledge of RC522, the exit button, or any other input source; it
 * doesn't call into app_rc522/app_persist/app_exit_button and doesn't
 * include their headers. Every one of those callers "issues a command"
 * by calling this one function - app_lock doesn't care, and can't tell,
 * who called it or why.
 *
 * (The project's convention for sharing state between app_ modules is
 * an accessor function, not a public global - see app_rtc/app_rc522's
 * get_latest/consume_if_updated pattern - so "a command variable" here
 * is a function call that sets one internally, not an extern bool other
 * modules poke directly. Same intent - any caller can request open
 * without app_lock needing to know who they are - different mechanism,
 * for the same reason those other modules don't expose raw globals.)
 *
 * Current callers (see 1.Document/docs/firmware/app-lock.md's "data
 * input" table for the full picture):
 *   - app_system (checks RC522 UID against app_persist - UC-02)
 *   - app_exit_button (EXTI-driven event handler - UC-03)
 * Neither is app_lock's concern; a 3rd future caller (e.g. a PIN-entry
 * app_system path, UC-01) needs zero change here.
 */

#pragma once

#include <stdbool.h>

typedef enum
{
    APP_LOCK_STATE_LOCKED,
    APP_LOCK_STATE_UNLOCKED
}
app_lock_state_t;

void app_lock_init(void);

/* Scheduler task - see app_main.c's g_pTaskTable. Closes the lock once
 * APP_LOCK_OPEN_DURATION_MS has elapsed since the last request_open(). */
void app_lock_task(void);

/* THE command: "open the lock for APP_LOCK_OPEN_DURATION_MS, starting
 * now". Safe to call from LOCKED (opens it) or UNLOCKED (just restarts
 * the window - this is what makes a 2nd card scan or exit-button press
 * during the open window extend it instead of doing nothing or erroring). */
void app_lock_request_open(void);

/* For a future LCD/status consumer - not polled by anything yet. */
app_lock_state_t app_lock_get_state(void);
