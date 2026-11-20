/*
 * app_persist.c
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "app_persist.h"
#include "bsp_eeprom.h"
#include "define.h"

#define APP_PERSIST_EEPROM_BASE_ADDR  (0x0000u)
#define APP_PERSIST_VERSION           (2u)  /* v1 had APP_PERSIST_MAX_USERS=32 - different blob
                                              * size/layout, bumped so an old v1 blob fails this
                                              * version check and self-heals instead of being
                                              * misread at the wrong offsets (see app_persist.h) */

static const uint8_t s_pui8Magic[4] = { 'A', 'C', 'P', 'S' };

typedef struct __attribute__((packed))
{
    uint8_t ui8Uid[RC522_UID_LEN];
    char    szName[APP_PERSIST_NAME_LEN];
    uint8_t ui8Valid;   /* 0/1 - plain uint8_t (not bool) to pin the on-disk byte layout */
}
app_persist_user_rec_t;

typedef struct __attribute__((packed))
{
    uint8_t                pui8Magic[4];
    uint8_t                 ui8Version;
    char                    szAdminPassword[APP_PERSIST_ADMIN_PASSWORD_LEN];
    uint8_t                 ui8UserCount;
    app_persist_user_rec_t  pUsers[APP_PERSIST_MAX_USERS];
    uint16_t                ui16Checksum;
}
app_persist_data_t;

static app_persist_data_t s_data;

/* CRC16-CCITT (poly 0x1021, init 0xFFFF) - bit-by-bit, fine for a ~689
 * byte blob touched only on init and on rare admin actions, not worth
 * a lookup table for this. */
static uint16_t app_persist_crc16(const uint8_t *pui8Data, uint32_t ui32Length)
{
    uint16_t ui16Crc = 0xFFFFU;

    for (uint32_t i = 0U; i < ui32Length; i++) {
        ui16Crc ^= (uint16_t)((uint16_t)pui8Data[i] << 8U);

        for (uint8_t bit = 0U; bit < 8U; bit++) {
            ui16Crc = ((ui16Crc & 0x8000U) != 0U) ? (uint16_t)((uint16_t)(ui16Crc << 1U) ^ 0x1021U)
                                                   : (uint16_t)(ui16Crc << 1U);
        }
    }

    return ui16Crc;
}

static void app_persist_set_defaults(void)
{
    memset(&s_data, 0, sizeof(s_data));
    memcpy(s_data.pui8Magic, s_pui8Magic, sizeof(s_pui8Magic));
    s_data.ui8Version = APP_PERSIST_VERSION;
    strncpy(s_data.szAdminPassword, "1234", APP_PERSIST_ADMIN_PASSWORD_LEN - 1U);
    s_data.ui8UserCount = 0U;
    /* pUsers[] already zeroed by memset above -> every ui8Valid == 0 */
}

/* Formats the ENTIRE blob - the only legitimate reason to write all
 * 2117 bytes at once (everything actually changed). Used solely by
 * app_persist_init()'s self-heal path and app_persist_factory_reset().
 * Every other mutation below writes only the specific bytes that
 * changed via app_persist_write_range() - see app_persist.h's top
 * comment. */
static void app_persist_flush_all(void)
{
    s_data.ui16Checksum = app_persist_crc16((const uint8_t *)&s_data,
                                             sizeof(s_data) - sizeof(s_data.ui16Checksum));

    bsp_eeprom_write(APP_PERSIST_EEPROM_BASE_ADDR, (const uint8_t *)&s_data, sizeof(s_data));
}

/* Writes ui32Length bytes starting at pData to EEPROM, at the offset
 * that same data occupies within s_data (ui32OffsetInBlob is normally
 * an offsetof(app_persist_data_t, field) expression, optionally plus an
 * array-element stride - see call sites). pData always points inside
 * s_data, so what's written is always this call's int32Length-byte
 * slice of the already-updated in-RAM copy, never stale data. */
static void app_persist_write_range(uint32_t ui32OffsetInBlob, const void *pData, uint32_t ui32Length)
{
    bsp_eeprom_write((uint16_t)(APP_PERSIST_EEPROM_BASE_ADDR + ui32OffsetInBlob),
                      (const uint8_t *)pData, ui32Length);
}

static void app_persist_write_user_slot(uint32_t ui32Index)
{
    uint32_t ui32Offset = (uint32_t)offsetof(app_persist_data_t, pUsers) +
                           (ui32Index * sizeof(app_persist_user_rec_t));

    app_persist_write_range(ui32Offset, &s_data.pUsers[ui32Index], sizeof(s_data.pUsers[ui32Index]));
}

static void app_persist_write_user_count(void)
{
    app_persist_write_range((uint32_t)offsetof(app_persist_data_t, ui8UserCount),
                             &s_data.ui8UserCount, sizeof(s_data.ui8UserCount));
}

static void app_persist_write_admin_password(void)
{
    app_persist_write_range((uint32_t)offsetof(app_persist_data_t, szAdminPassword),
                             &s_data.szAdminPassword, sizeof(s_data.szAdminPassword));
}

/* Recomputes the CRC over the whole in-RAM struct (cheap, pure CPU) and
 * writes only the 2 checksum bytes - correct as long as every other
 * field changed this call already had its own targeted write done
 * first, so EEPROM already matches RAM everywhere except this field. */
static void app_persist_update_checksum(void)
{
    s_data.ui16Checksum = app_persist_crc16((const uint8_t *)&s_data,
                                             sizeof(s_data) - sizeof(s_data.ui16Checksum));

    app_persist_write_range((uint32_t)offsetof(app_persist_data_t, ui16Checksum),
                             &s_data.ui16Checksum, sizeof(s_data.ui16Checksum));
}

/* -1 if no valid slot holds this UID. */
static int32_t app_persist_find_slot_index(const uint8_t pui8Uid[RC522_UID_LEN])
{
    for (uint32_t i = 0U; i < APP_PERSIST_MAX_USERS; i++) {
        if ((s_data.pUsers[i].ui8Valid != 0U) &&
            (memcmp(s_data.pUsers[i].ui8Uid, pui8Uid, RC522_UID_LEN) == 0)) {
            return (int32_t)i;
        }
    }

    return -1;
}

void app_persist_init(void)
{
    bsp_eeprom_init();

    bsp_eeprom_read(APP_PERSIST_EEPROM_BASE_ADDR, (uint8_t *)&s_data, sizeof(s_data));

    bool bMagicOk = (memcmp(s_data.pui8Magic, s_pui8Magic, sizeof(s_pui8Magic)) == 0);
    bool bVersionOk = (s_data.ui8Version == APP_PERSIST_VERSION);
    uint16_t ui16ExpectedCrc = app_persist_crc16((const uint8_t *)&s_data,
                                                  sizeof(s_data) - sizeof(s_data.ui16Checksum));
    bool bCrcOk = (s_data.ui16Checksum == ui16ExpectedCrc);

    if (!bMagicOk || !bVersionOk || !bCrcOk) {
        /* First boot, or EEPROM erased/corrupted/wrong layout version -
         * self-heal rather than run off garbage data. */
        app_persist_set_defaults();
        app_persist_flush_all();
    }
}

void app_persist_factory_reset(void)
{
    app_persist_set_defaults();
    app_persist_flush_all();
}

bool app_persist_admin_password_check(const char *pszPassword)
{
    if (pszPassword == NULL) {
        return false;
    }

    return (strcmp(s_data.szAdminPassword, pszPassword) == 0);
}

uint32_t app_persist_admin_password_set(const char *pszNewPassword)
{
    if (pszNewPassword == NULL) {
        return ERROR_INVALID_PARAM;
    }

    if (strlen(pszNewPassword) > (APP_PERSIST_ADMIN_PASSWORD_LEN - 1U)) {
        return ERROR_INVALID_PARAM;
    }

    memset(s_data.szAdminPassword, 0, APP_PERSIST_ADMIN_PASSWORD_LEN);
    strncpy(s_data.szAdminPassword, pszNewPassword, APP_PERSIST_ADMIN_PASSWORD_LEN - 1U);

    app_persist_write_admin_password();
    app_persist_update_checksum();

    return ERROR_OK;
}

uint32_t app_persist_user_add(const uint8_t pui8Uid[RC522_UID_LEN], const char *pszName)
{
    if ((pui8Uid == NULL) || (pszName == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    if (app_persist_find_slot_index(pui8Uid) >= 0) {
        return ERROR_FAIL; /* UID already registered */
    }

    uint32_t ui32FreeIndex = APP_PERSIST_MAX_USERS;

    for (uint32_t i = 0U; i < APP_PERSIST_MAX_USERS; i++) {
        if (s_data.pUsers[i].ui8Valid == 0U) {
            ui32FreeIndex = i;
            break;
        }
    }

    if (ui32FreeIndex == APP_PERSIST_MAX_USERS) {
        return ERROR_BUSY; /* table full - see APP_PERSIST_MAX_USERS */
    }

    app_persist_user_rec_t *pSlot = &s_data.pUsers[ui32FreeIndex];

    memcpy(pSlot->ui8Uid, pui8Uid, RC522_UID_LEN);
    memset(pSlot->szName, 0, APP_PERSIST_NAME_LEN);
    strncpy(pSlot->szName, pszName, APP_PERSIST_NAME_LEN - 1U);
    pSlot->ui8Valid = 1U;

    s_data.ui8UserCount++;

    app_persist_write_user_slot(ui32FreeIndex);
    app_persist_write_user_count();
    app_persist_update_checksum();

    return ERROR_OK;
}

uint32_t app_persist_user_remove(const uint8_t pui8Uid[RC522_UID_LEN])
{
    if (pui8Uid == NULL) {
        return ERROR_INVALID_PARAM;
    }

    int32_t i32Index = app_persist_find_slot_index(pui8Uid);

    if (i32Index < 0) {
        return ERROR_FAIL; /* not found */
    }

    uint32_t ui32Index = (uint32_t)i32Index;

    memset(&s_data.pUsers[ui32Index], 0, sizeof(s_data.pUsers[ui32Index]));
    s_data.ui8UserCount--;

    app_persist_write_user_slot(ui32Index);
    app_persist_write_user_count();
    app_persist_update_checksum();

    return ERROR_OK;
}

bool app_persist_user_find_by_uid(const uint8_t pui8Uid[RC522_UID_LEN], char *pszNameOut)
{
    if ((pui8Uid == NULL) || (pszNameOut == NULL)) {
        return false;
    }

    int32_t i32Index = app_persist_find_slot_index(pui8Uid);

    if (i32Index < 0) {
        return false;
    }

    memcpy(pszNameOut, s_data.pUsers[i32Index].szName, APP_PERSIST_NAME_LEN);

    return true;
}

uint32_t app_persist_user_count(void)
{
    return s_data.ui8UserCount;
}

bool app_persist_user_get_by_index(uint32_t ui32Index,
                                    uint8_t pui8Uid[RC522_UID_LEN], char *pszNameOut)
{
    if ((pui8Uid == NULL) || (pszNameOut == NULL)) {
        return false;
    }

    uint32_t ui32Seen = 0U;

    for (uint32_t i = 0U; i < APP_PERSIST_MAX_USERS; i++) {
        if (s_data.pUsers[i].ui8Valid == 0U) {
            continue;
        }

        if (ui32Seen == ui32Index) {
            memcpy(pui8Uid, s_data.pUsers[i].ui8Uid, RC522_UID_LEN);
            memcpy(pszNameOut, s_data.pUsers[i].szName, APP_PERSIST_NAME_LEN);
            return true;
        }

        ui32Seen++;
    }

    return false;
}
