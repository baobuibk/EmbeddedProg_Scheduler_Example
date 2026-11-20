/*
 * app_rc522.h
 *
 * app_rc522_task(): a periodic scheduler task (see app_main.c's
 * g_pTaskTable) that asks "is a card there" (bsp_rc522_poll(), cheap
 * REQA) and, only on a real answer, immediately reads the full UID
 * (bsp_rc522_read_uid()) right in the same call - no separate
 * event/interrupt path. That split was tried first (RC522's IRQ pin
 * posting a Dev/4.MiddleWare/event, read done from the event handler)
 * and dropped: this board's scheduler is cooperative and
 * bsp_rc522_poll() already blocks synchronously on the SPI wait, so by
 * the time it returns, nothing was gained by waiting for an interrupt
 * instead - no other task could have run in between either way, and
 * the event path even forced a second, redundant REQA. See
 * Dev/2.BSP/bsp_rc522.h's top comment for the same reasoning from the
 * BSP side, and under what condition (a genuinely async rc522.c) it
 * would be worth revisiting.
 *
 * Caches the last UID read the same way app_rtc caches the last time
 * (Dev/1.Application/app_rtc) - a sequence number + two accessors, one
 * for "what's the last UID" on demand (e.g. an access-control decision
 * made elsewhere), one for "has a NEW UID shown up since I last looked"
 * (e.g. logging each scan exactly once). On a successful read, the card
 * is also put back to HALT (bsp_rc522_halt()) so it stops answering
 * REQA while still sitting in the field - without that, the same card
 * would keep re-triggering a fresh "read" every poll cycle.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "rc522.h"

void app_rc522_init(void);

/* Scheduler task - see app_main.c's g_pTaskTable. Polls, and on a
 * successful REQA, reads and caches the UID in the same call. */
void app_rc522_task(void);

/* Copies the last successfully-read UID into *pui8Uid (and *pui8Sak if
 * non-NULL). Returns false (leaving outputs untouched) if no card has
 * been read successfully yet. */
bool app_rc522_get_last_uid(uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak);

/* *pui32LastSeenSeq: caller-owned, zero-initialize before the first
 * call - same pattern as app_rtc_consume_if_updated(). If a new UID has
 * been read since *pui32LastSeenSeq was last updated, copies it into
 * *pui8Uid, updates *pui32LastSeenSeq to match, and returns true.
 * Returns false (nothing touched) otherwise. */
bool app_rc522_consume_if_updated(uint8_t pui8Uid[RC522_UID_LEN], uint32_t *pui32LastSeenSeq);
