/*
 * app_rtc.h
 *
 * Periodically re-reads the DS1307 (every 1s - see app_rtc_task's entry
 * in Dev/1.Application/app_main.c's g_pTaskTable) through bsp_rtc and
 * caches the result here, as the single source of truth for "what time
 * is it" - so the LCD-display and EEPROM-log consumers don't each need
 * their own I2C1 transaction. I2C1 is shared with the EEPROM and LCD
 * backpack and is comparatively slow, so one read/second here beats
 * every consumer polling the chip independently.
 *
 * Two ways to consume the cached time, matching this module's two
 * actual consumers:
 *
 *   - app_rtc_get_time(): "what time is it right now" - for a consumer
 *     that just needs a timestamp on demand, e.g. the EEPROM log
 *     (UC-09) stamping an access event whenever it happens, not on any
 *     particular schedule of its own.
 *
 *   - app_rtc_consume_if_updated(): "has the time actually changed since
 *     I last looked" - for a consumer that must redraw only on a real
 *     change instead of every time it happens to run, e.g. the LCD
 *     clock display. Each caller owns its own uint32_t sequence number
 *     (zero-initialize it once, e.g. as a static local) and passes its
 *     address in on every call; independent callers never collide
 *     since each tracks its own last-seen value against the one shared
 *     counter here.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ds1307.h"

void app_rtc_init(void);

/* Scheduler task - see app_main.c's g_pTaskTable (every 1000 ticks =
 * 1s, assuming the scheduler's own 1000 ticks/sec rate). */
void app_rtc_task(void);

/* Copies the last successfully-read time into *pTime.
 * Returns false (leaving *pTime untouched) if the RTC hasn't been read
 * successfully even once yet. */
bool app_rtc_get_time(ds1307_time_t *pTime);

/* *pui32LastSeenSeq: caller-owned, zero-initialize before the first
 * call. If the cached time has been refreshed since *pui32LastSeenSeq
 * was last updated, copies it into *pTime, updates *pui32LastSeenSeq to
 * match, and returns true. Returns false (nothing touched) otherwise,
 * including before the first successful RTC read. */
bool app_rtc_consume_if_updated(ds1307_time_t *pTime, uint32_t *pui32LastSeenSeq);
