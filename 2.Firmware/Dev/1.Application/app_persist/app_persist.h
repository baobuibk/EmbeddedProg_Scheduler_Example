/*
 * app_persist.h
 *
 * Persistent storage for this system's admin/user data in the 24C256
 * EEPROM (via Dev/2.BSP/bsp_eeprom) - admin (master) password, and the
 * UID + display-name list used by RC522 card validation (UC-02) and
 * admin menus (UC-06/UC-07) to show "who" a UID belongs to on the LCD.
 *
 * Unlike app_rtc/app_rc522 (periodic scheduler task re-reading fast,
 * changing hardware state every tick), this data changes rarely - only
 * on an explicit admin action - so the shape here is different: load
 * once into a RAM cache at app_persist_init(), serve every read
 * straight from RAM (no I2C traffic). No scheduler task needed.
 *
 * Every mutating call writes ONLY the bytes that actually changed - the
 * one touched user record (or the admin password field), the user
 * count, and the checksum - never the whole 2117-byte blob, computed by
 * offset into app_persist_data_t (see app_persist_write_range() in the
 * .c file). The *only* place that ever writes the entire blob is
 * app_persist_set_defaults()+flush, used by app_persist_init()'s
 * self-heal path and by app_persist_factory_reset() - both are
 * genuinely "format everything", not an update of one field. Every
 * targeted write still recomputes the CRC over the whole in-RAM struct
 * (cheap, no I2C) before writing just the 2 checksum bytes - this only
 * stays correct because EEPROM and RAM are kept in sync field-by-field
 * on every call, never left to drift.
 *
 * On-disk layout (single contiguous blob starting at EEPROM address 0,
 * see app_persist_data_t below - all packed, no compiler padding):
 *
 *   offset  size  field
 *   0       4     magic ("ACPS" literal bytes) - detects an unformatted
 *                 or wrong-layout EEPROM (first boot, or after a
 *                 firmware update that changed this struct)
 *   4       1     version (APP_PERSIST_VERSION) - bump this whenever
 *                 the layout changes, so a future app_persist can tell
 *                 an old-format blob apart from a corrupt one
 *   5       9     admin password (ASCII, NUL-padded, up to 8 chars)
 *   14      1     user count (number of occupied slots, <= MAX_USERS)
 *   15      2100  user table: APP_PERSIST_MAX_USERS x 21 bytes each
 *                 (4-byte UID + 16-byte name + 1-byte valid flag)
 *   2115    2     CRC16-CCITT over everything above
 *
 * Total 2117 bytes, far under the 24C256's 32768 - everything from
 * offset 2117 onward is free for a future EEPROM consumer (e.g. the
 * UC-09 access-log ring buffer) without colliding with this table.
 *
 * app_persist_init() validates magic+version+CRC on load; if any of
 * them don't match (first boot, or EEPROM erased/corrupted), it resets
 * the RAM cache to defaults (empty user table, a hardcoded default
 * admin password) and immediately writes that back - self-healing, and
 * incidentally exactly what app_persist_factory_reset() does too (UC-10).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "rc522.h"

#define APP_PERSIST_MAX_USERS           (100u) /* plenty of EEPROM room to raise this further */
#define APP_PERSIST_NAME_LEN            (16u)  /* display width of the 16x2 LCD, +NUL budget below */
#define APP_PERSIST_ADMIN_PASSWORD_LEN  (9u)   /* 8 chars + NUL */

/* Total on-disk blob size, computed (not hardcoded as "2117") from the
 * same public constants used above, so it can never silently go stale
 * the way a copy-pasted literal would if this layout changes again (as
 * it already did once, when APP_PERSIST_MAX_USERS went 32->100 and
 * every size/offset below the user table shifted). Any future EEPROM
 * consumer (e.g. app_log/UC-09) should start at this offset, never a
 * literal number. */
#define APP_PERSIST_EEPROM_SIZE_BYTES \
    (4u + 1u + APP_PERSIST_ADMIN_PASSWORD_LEN + 1u + \
     (APP_PERSIST_MAX_USERS * (RC522_UID_LEN + APP_PERSIST_NAME_LEN + 1u)) + 2u)

void app_persist_init(void);

/* Resets admin password + user table to factory defaults and writes
 * the result to EEPROM immediately (UC-10). */
void app_persist_factory_reset(void);

/* Admin (master) password - UC-08. */
bool app_persist_admin_password_check(const char *pszPassword);

/* ERROR_OK, or ERROR_INVALID_PARAM if pszNewPassword is NULL or longer
 * than APP_PERSIST_ADMIN_PASSWORD_LEN-1 chars. On success, writes only
 * the password field + checksum to EEPROM (not the whole blob). */
uint32_t app_persist_admin_password_set(const char *pszNewPassword);

/* Users (RC522 UID <-> display name) - UC-02/UC-06/UC-07.
 *
 * pszName is always NUL-terminated by this module on output, truncated
 * to APP_PERSIST_NAME_LEN-1 chars on input if longer.
 */

/* ERROR_OK, ERROR_INVALID_PARAM (NULL args), ERROR_FAIL (UID already
 * registered), or ERROR_BUSY (table full, see APP_PERSIST_MAX_USERS).
 * On success, writes only the one touched user record + user count +
 * checksum to EEPROM (not the whole blob). */
uint32_t app_persist_user_add(const uint8_t pui8Uid[RC522_UID_LEN], const char *pszName);

/* ERROR_OK, ERROR_INVALID_PARAM (NULL), or ERROR_FAIL (UID not found).
 * On success, writes only the one touched (now-cleared) user record +
 * user count + checksum to EEPROM (not the whole blob). */
uint32_t app_persist_user_remove(const uint8_t pui8Uid[RC522_UID_LEN]);

/* Looks up pui8Uid among registered users. On a match, copies the
 * display name into pszNameOut (caller-provided buffer, at least
 * APP_PERSIST_NAME_LEN bytes) and returns true. Returns false (buffer
 * untouched) if not found - this IS the UC-02 "so khớp UID" check. */
bool app_persist_user_find_by_uid(const uint8_t pui8Uid[RC522_UID_LEN], char *pszNameOut);

/* Number of occupied user slots - for an admin menu to know how many
 * entries app_persist_user_get_by_index() can return. */
uint32_t app_persist_user_count(void);

/* The ui32Index-th OCCUPIED slot (0-based, not a raw table index - skips
 * deleted/empty slots), for an admin menu to list/scroll through
 * registered users on the LCD. Returns false (outputs untouched) if
 * ui32Index >= app_persist_user_count(). */
bool app_persist_user_get_by_index(uint32_t ui32Index,
                                    uint8_t pui8Uid[RC522_UID_LEN], char *pszNameOut);
