/*
 * bsp_systick.c
 */

#include <stdint.h>

#include "bsp_systick.h"
#include "scheduler.h"
#include "stm32f1xx.h"

/*
 * Overrides scheduler.c's weak platform_systick_init() stub. Project
 * moved off HAL onto LL - SysTick is now started from scratch here
 * (no HAL_Init()/HAL_GetTick() running earlier to assume anything
 * about), using CMSIS SysTick registers directly (same registers LL's
 * own SysTick helpers would use - this file just writes them without
 * going through an LL wrapper, since there's no LL_SYSTICK module,
 * only the CMSIS core_cm3.h-level ones). Configures the rate the
 * scheduler actually asked for (normally 1ms/1000Hz).
 */
void platform_systick_init(uint32_t TicksPerSecond)
{
    extern uint32_t SystemCoreClock;

    if (TicksPerSecond == 0U) {
        return;
    }

    uint32_t reload = (SystemCoreClock / TicksPerSecond) - 1U;

    if (reload > SysTick_LOAD_RELOAD_Msk) {
        return; /* TicksPerSecond too low for a 24-bit reload value */
    }

    SysTick->CTRL = 0U; /* disable while reconfiguring */
    SysTick->LOAD = reload;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}
