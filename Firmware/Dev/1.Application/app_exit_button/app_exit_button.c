/*
 * app_exit_button.c
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "app_exit_button.h"
#include "bsp_exit_button.h"
#include "app_lock.h"
#include "app_system.h"
#include "app_log.h"
#include "os.h"

/* Contact bounce on PA11 can make EXTI11 fire several times for one
 * physical press, each landing in a separate event_run() pass (not
 * necessarily coalesced into a single pending flag - see app_lock.h's
 * old note on why that used to be considered harmless anyway). Ignore
 * any press within this window of the last ACCEPTED one, so one real
 * press only ever runs the three calls below once. */
#define APP_EXIT_BUTTON_DEBOUNCE_MS  (200u)

static uint32_t s_ui32LastAcceptedTick = 0U;
static bool s_bHasAcceptedBefore = false;

void app_exit_button_init(void)
{
    bsp_exit_button_init();
}

void app_exit_button_on_press(void)
{
    uint32_t ui32Now = os_tick_get();

    if (s_bHasAcceptedBefore && ((ui32Now - s_ui32LastAcceptedTick) < APP_EXIT_BUTTON_DEBOUNCE_MS)) {
        return; /* debounce - almost certainly contact bounce from the same press */
    }

    s_ui32LastAcceptedTick = ui32Now;
    s_bHasAcceptedBefore = true;

    app_lock_request_open();
    app_system_notify_exit_pressed(); /* display feedback only - app_lock already opened above
                                        * regardless of app_system's state, see app_system.h */
    app_log_record(APP_LOG_SOURCE_EXIT, NULL); /* unconditional, unlike the notify above - the
                                                 * exit event happened whether or not app_system
                                                 * was IDLE to show it, UC-09 */
}
