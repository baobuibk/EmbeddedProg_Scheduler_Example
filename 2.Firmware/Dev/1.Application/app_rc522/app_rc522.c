/*
 * app_rc522.c
 */

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "app_rc522.h"
#include "bsp_rc522.h"
#include "rc522.h"
#include "define.h"

static uint8_t s_pui8LastUid[RC522_UID_LEN];
static uint8_t s_ui8LastSak;
static uint32_t s_ui32UpdateSeq = 0U;
static bool s_bHasUid = false;

void app_rc522_init(void)
{
    bsp_rc522_init();
}

void app_rc522_task(void)
{
    if (bsp_rc522_poll() != ERROR_OK) {
        return; /* no card answered REQA */
    }

    uint8_t pui8Uid[RC522_UID_LEN];
    uint8_t ui8Sak;

    if (bsp_rc522_read_uid(pui8Uid, &ui8Sak) == ERROR_OK) {
        memcpy(s_pui8LastUid, pui8Uid, RC522_UID_LEN);
        s_ui8LastSak = ui8Sak;
        s_ui32UpdateSeq++;
        s_bHasUid = true;

        bsp_rc522_halt(); /* stop the same card re-triggering this every poll */
    }
}

bool app_rc522_get_last_uid(uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak)
{
    if ((pui8Uid == NULL) || !s_bHasUid) {
        return false;
    }

    memcpy(pui8Uid, s_pui8LastUid, RC522_UID_LEN);

    if (pui8Sak != NULL) {
        *pui8Sak = s_ui8LastSak;
    }

    return true;
}

bool app_rc522_consume_if_updated(uint8_t pui8Uid[RC522_UID_LEN], uint32_t *pui32LastSeenSeq)
{
    if ((pui8Uid == NULL) || (pui32LastSeenSeq == NULL) || !s_bHasUid) {
        return false;
    }

    if (*pui32LastSeenSeq == s_ui32UpdateSeq) {
        return false;
    }

    memcpy(pui8Uid, s_pui8LastUid, RC522_UID_LEN);
    *pui32LastSeenSeq = s_ui32UpdateSeq;

    return true;
}
