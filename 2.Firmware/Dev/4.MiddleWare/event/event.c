/*
 * event.c
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "event.h"

void event_post(uint32_t ui32EventId)
{
    if (ui32EventId < g_ui32EventNumEvents) {
        g_pEventTable[ui32EventId].bPending = true;
    }
}

void event_run(void)
{
    for (uint32_t ui32Index = 0U; ui32Index < g_ui32EventNumEvents; ui32Index++) {
        tEventEntry *pEvent = &g_pEventTable[ui32Index];

        if (pEvent->bPending) {
            pEvent->bPending = false; /* xoá trước khi gọi - xem event.h */

            if (pEvent->pfnHandler != NULL) {
                pEvent->pfnHandler();
            }
        }
    }
}
