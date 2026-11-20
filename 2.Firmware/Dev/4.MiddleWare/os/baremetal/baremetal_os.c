/*
 * baremetal_os.c
 *
 * Bare-metal (no RTOS) backend for the os.h abstraction.
 *
 * Timing comes from the cooperative scheduler's own tick counter
 * (Dev/4.MiddleWare/scheduler), not a vendor HAL tick - that keeps this
 * module swappable: an RTOS backend (os/rtos/rtos_os.c, built on e.g.
 * FreeRTOS/CMSIS-RTOS primitives) can replace this file later without
 * spi_io.c, stm32_spi_io.c, or any other caller changing at all.
 *
 * os_lock_acquire() spins rather than blocks a task, because a bare-metal
 * build has no scheduler to switch away to while waiting; interrupts are
 * only masked for the few instructions needed to test-and-set the flag,
 * so the critical section stays short regardless of the timeout value.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "os.h"
#include "define.h"
#include "scheduler.h"
#include "stm32f1xx.h"

bool os_lock_init(os_lock_t *lock)
{
    if (lock == NULL) {
        return false;
    }

    lock->locked = 0U;
    return true;
}

uint32_t os_lock_acquire(os_lock_t *lock, uint32_t timeout_ms)
{
    if (lock == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint32_t start = scheduler_tick_count_get();

    for (;;) {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        bool was_locked = (lock->locked != 0U);
        if (!was_locked) {
            lock->locked = 1U;
        }

        if (primask == 0U) {
            __enable_irq();
        }

        if (!was_locked) {
            return ERROR_OK;
        }

        if (scheduler_elapsed_ticks_get(start) >= timeout_ms) {
            return ERROR_TIMEOUT;
        }
    }
}

uint32_t os_lock_release(os_lock_t *lock)
{
    if (lock == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    lock->locked = 0U;
    if (primask == 0U) {
        __enable_irq();
    }

    return ERROR_OK;
}

uint32_t os_reg_wait_flag_blocking(const volatile uint32_t *reg, uint32_t mask, uint32_t timeout_ms)
{
    if (reg == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint32_t start = scheduler_tick_count_get();

    while ((*reg & mask) == 0U) {
        if (scheduler_elapsed_ticks_get(start) >= timeout_ms) {
            return ERROR_TIMEOUT;
        }
    }

    return ERROR_OK;
}

uint32_t os_reg_wait_flag_clear_blocking(const volatile uint32_t *reg, uint32_t mask, uint32_t timeout_ms)
{
    if (reg == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint32_t start = scheduler_tick_count_get();

    while ((*reg & mask) != 0U) {
        if (scheduler_elapsed_ticks_get(start) >= timeout_ms) {
            return ERROR_TIMEOUT;
        }
    }

    return ERROR_OK;
}

uint32_t os_tick_get(void)
{
    return scheduler_tick_count_get();
}
