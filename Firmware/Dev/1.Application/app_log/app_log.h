/*
 * app_log.h
 *
 * UC-09: access-log ring buffer in the 24C256 EEPROM, right after
 * app_persist's own region (`APP_PERSIST_EEPROM_SIZE_BYTES` from
 * app_persist.h - never a hardcoded offset, see that header's own
 * comment for why). Every record is timestamped via app_rtc_get_time()
 * (get_latest, not consume_if_updated - a log reacts to its own event,
 * it doesn't poll, matching app_rtc.h's own documented rationale for
 * that accessor).
 *
 * No scheduler task - app_log_record() is called directly by whatever
 * app_* module just decided something log-worthy happened
 * (app_system for RFID grant/deny, app_exit_button for the exit
 * button), the same way app_persist's mutators are called directly
 * rather than polled.
 *
 * Deliberately simpler than app_persist: no CRC, no targeted-write
 * dance for the ring buffer itself - losing/corrupting one log record
 * on a power cut mid-write is an acceptable risk for a log (unlike
 * app_persist's user table, where losing data on a half-write was
 * worth guarding against). Only the header's write-index field uses a
 * targeted write (4 bytes), to avoid rewriting the whole header on
 * every single log entry.
 */

#pragma once

#include <stdint.h>

#include "rc522.h"

typedef enum
{
    APP_LOG_SOURCE_RFID_GRANTED,
    APP_LOG_SOURCE_RFID_DENIED,
    APP_LOG_SOURCE_EXIT
}
app_log_source_t;

void app_log_init(void);

/* pui8Uid: NULL for APP_LOG_SOURCE_EXIT (no card involved) - stored as
 * all-zero UID in that case. Timestamps with whatever app_rtc currently
 * has cached; if RTC has never been read successfully, stores an
 * all-zero timestamp rather than skipping the record (still records
 * that *something* happened, just without a reliable time). */
void app_log_record(app_log_source_t eSource, const uint8_t pui8Uid[RC522_UID_LEN]);
