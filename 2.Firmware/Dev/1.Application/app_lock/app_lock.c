/*
 * app_lock.c
 */

#include <stdint.h>

#include "app_lock.h"
#include "bsp_lock.h"
#include "os.h"

#define APP_LOCK_OPEN_DURATION_MS  (5000u)  /* 5s - see app_lock.h/app-lock.md */

static app_lock_state_t s_state = APP_LOCK_STATE_LOCKED;
static uint32_t s_ui32UnlockTick = 0U;

void app_lock_init(void)
{
    bsp_lock_init();
    s_state = APP_LOCK_STATE_LOCKED;
}

void app_lock_task(void)
{
    if (s_state != APP_LOCK_STATE_UNLOCKED) {
        return;
    }

    if ((os_tick_get() - s_ui32UnlockTick) >= APP_LOCK_OPEN_DURATION_MS) {
        bsp_lock_close();
        s_state = APP_LOCK_STATE_LOCKED;
    }
}

void app_lock_request_open(void)
{
    if (s_state == APP_LOCK_STATE_LOCKED) {
        bsp_lock_open();
        s_state = APP_LOCK_STATE_UNLOCKED;
    }

    /* Always restart the 5s window, whether this just opened the lock
     * or it was already open - see app_lock.h's top comment. */
    s_ui32UnlockTick = os_tick_get();
}

app_lock_state_t app_lock_get_state(void)
{
    return s_state;
}
