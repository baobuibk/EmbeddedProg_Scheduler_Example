/*
 * app_log.c
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "app_log.h"
#include "bsp_eeprom.h"
#include "app_persist.h"
#include "app_rtc.h"
#include "ds1307.h"
#include "rc522.h"

#define APP_LOG_MAX_RECORDS  (2000u)  /* 2000*13 = 26000 bytes, well under the ~30.6KB free
                                        * after app_persist's region */

static const uint8_t s_pui8Magic[4] = { 'A', 'L', 'O', 'G' };

typedef struct __attribute__((packed))
{
    uint8_t  ui8Seconds;
    uint8_t  ui8Minutes;
    uint8_t  ui8Hours;
    uint8_t  ui8Day;
    uint8_t  ui8Date;
    uint8_t  ui8Month;
    uint16_t ui16Year;
    uint8_t  ui8Source;              /* app_log_source_t */
    uint8_t  ui8Uid[RC522_UID_LEN];  /* all-zero for APP_LOG_SOURCE_EXIT */
}
app_log_record_t;  /* 13 bytes */

typedef struct __attribute__((packed))
{
    uint8_t  pui8Magic[4];
    uint32_t ui32NextIndex;  /* next slot to write, wraps mod APP_LOG_MAX_RECORDS */
}
app_log_header_t;  /* 8 bytes */

#define APP_LOG_HEADER_ADDR   (APP_PERSIST_EEPROM_SIZE_BYTES)
#define APP_LOG_RECORDS_ADDR  (APP_LOG_HEADER_ADDR + (uint32_t)sizeof(app_log_header_t))

static app_log_header_t s_header;

void app_log_init(void)
{
    bsp_eeprom_read(APP_LOG_HEADER_ADDR, (uint8_t *)&s_header, sizeof(s_header));

    if (memcmp(s_header.pui8Magic, s_pui8Magic, sizeof(s_pui8Magic)) != 0) {
        /* First boot, or this region never formatted - start a fresh
         * ring buffer rather than trust garbage. */
        memcpy(s_header.pui8Magic, s_pui8Magic, sizeof(s_pui8Magic));
        s_header.ui32NextIndex = 0U;
        bsp_eeprom_write(APP_LOG_HEADER_ADDR, (const uint8_t *)&s_header, sizeof(s_header));
    }
}

void app_log_record(app_log_source_t eSource, const uint8_t pui8Uid[RC522_UID_LEN])
{
    app_log_record_t rec;
    memset(&rec, 0, sizeof(rec));

    ds1307_time_t time;

    if (app_rtc_get_time(&time)) {
        rec.ui8Seconds = time.ui8Seconds;
        rec.ui8Minutes = time.ui8Minutes;
        rec.ui8Hours = time.ui8Hours;
        rec.ui8Day = time.ui8Day;
        rec.ui8Date = time.ui8Date;
        rec.ui8Month = time.ui8Month;
        rec.ui16Year = time.ui16Year;
    }
    /* else: RTC never read successfully yet - rec stays all-zero
     * timestamp, see app_log.h's top comment. */

    rec.ui8Source = (uint8_t)eSource;

    if (pui8Uid != NULL) {
        memcpy(rec.ui8Uid, pui8Uid, RC522_UID_LEN);
    }

    uint32_t ui32Slot = s_header.ui32NextIndex % APP_LOG_MAX_RECORDS;
    uint32_t ui32Addr = APP_LOG_RECORDS_ADDR + (ui32Slot * (uint32_t)sizeof(app_log_record_t));

    bsp_eeprom_write((uint16_t)ui32Addr, (const uint8_t *)&rec, sizeof(rec));

    s_header.ui32NextIndex++;

    /* Targeted write - only the 4-byte index, not the whole header. */
    bsp_eeprom_write((uint16_t)(APP_LOG_HEADER_ADDR + (uint32_t)offsetof(app_log_header_t, ui32NextIndex)),
                      (const uint8_t *)&s_header.ui32NextIndex, sizeof(s_header.ui32NextIndex));
}
