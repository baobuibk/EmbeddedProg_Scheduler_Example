/*
 * app_rtc.c
 */

#include <stddef.h>
#include <stdbool.h>

#include "app_rtc.h"
#include "bsp_rtc.h"
#include "ds1307.h"
#include "define.h"

static ds1307_time_t s_currentTime;
static uint32_t s_ui32UpdateSeq = 0U;
static bool s_bHasTime = false;

void app_rtc_init(void)
{
    bsp_rtc_init();
}

void app_rtc_task(void)
{
    ds1307_time_t time;

    if (bsp_rtc_get_time(&time) == ERROR_OK) {
        s_currentTime = time;
        s_ui32UpdateSeq++;
        s_bHasTime = true;
    }
}

bool app_rtc_get_time(ds1307_time_t *pTime)
{
    if ((pTime == NULL) || !s_bHasTime) {
        return false;
    }

    *pTime = s_currentTime;

    return true;
}

bool app_rtc_consume_if_updated(ds1307_time_t *pTime, uint32_t *pui32LastSeenSeq)
{
    if ((pTime == NULL) || (pui32LastSeenSeq == NULL) || !s_bHasTime) {
        return false;
    }

    if (*pui32LastSeenSeq == s_ui32UpdateSeq) {
        return false;
    }

    *pTime = s_currentTime;
    *pui32LastSeenSeq = s_ui32UpdateSeq;

    return true;
}
