/*
 * os.h
 *
 * Minimal OS abstraction used by platform-agnostic drivers (see
 * Dev/3.Drivers/SPI/spi_io.h for the main consumer): a mutual-exclusion
 * lock and a register-polling helper, both expressed in terms of a tick
 * count rather than any specific timer/HAL, so drivers never reference a
 * vendor HAL or RTOS type directly.
 *
 * Exactly one backend is compiled in under os/<backend>/:
 *   - os/baremetal/baremetal_os.c  - no RTOS; os_lock_acquire() spins and
 *     timing is driven by Dev/4.MiddleWare/scheduler's own tick count.
 *   - a future os/rtos/rtos_os.c   - would back the same API with real
 *     RTOS primitives (e.g. a FreeRTOS/CMSIS-RTOS mutex and osDelay-style
 *     wait), without spi_io.c or any other caller changing at all.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @struct os_lock_t
 * @brief Opaque mutual-exclusion lock handle; layout is backend-specific.
 */
typedef struct
{
    volatile uint8_t locked;
} os_lock_t;

/**
 * @brief Initialize a lock to the unlocked state.
 * @return true on success, false if lock is NULL.
 */
bool os_lock_init(os_lock_t *lock);

/**
 * @brief Acquire the lock, waiting up to timeout_ms if it is already held.
 * @return ERROR_OK on success, ERROR_TIMEOUT if not acquired in time,
 *         ERROR_INVALID_PARAM if lock is NULL.
 */
uint32_t os_lock_acquire(os_lock_t *lock, uint32_t timeout_ms);

/**
 * @brief Release a previously-acquired lock.
 * @return ERROR_OK on success, ERROR_INVALID_PARAM if lock is NULL.
 */
uint32_t os_lock_release(os_lock_t *lock);

/**
 * @brief Poll a hardware register until (*reg & mask) != 0, or time out.
 *
 * Lets drivers wait on status flags (TX/RX ready, etc.) without baking a
 * platform-specific timebase into the driver itself.
 *
 * @return ERROR_OK if the flag became set in time, ERROR_TIMEOUT otherwise,
 *         ERROR_INVALID_PARAM if reg is NULL.
 */
uint32_t os_reg_wait_flag_blocking(const volatile uint32_t *reg, uint32_t mask, uint32_t timeout_ms);

/**
 * @brief Poll a hardware register until (*reg & mask) == 0, or time out.
 *
 * The complement of os_reg_wait_flag_blocking(), for status bits a driver
 * must wait to clear (e.g. an I2C "bus busy" flag before issuing START).
 *
 * @return ERROR_OK if the flag cleared in time, ERROR_TIMEOUT otherwise,
 *         ERROR_INVALID_PARAM if reg is NULL.
 */
uint32_t os_reg_wait_flag_clear_blocking(const volatile uint32_t *reg, uint32_t mask, uint32_t timeout_ms);

/**
 * @brief Current tick count, in the same units os_*_acquire/wait timeouts
 *        are expressed in (milliseconds, on the bare-metal backend).
 */
uint32_t os_tick_get(void);
