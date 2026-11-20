/*
 * i2c_io.h
 *
 *  Platform-agnostic I2C bus abstraction, mirroring the structure of
 *  Dev/3.Drivers/SPI/spi_io.h: this header declares the public API, and
 *  exactly one backend under i2c/<platform>/ implements it (see
 *  stm32/stm32_i2c_io.c for the STM32F103 backend used by this project).
 *
 *  Notes:
 *  - OS abstraction uses opaque lock handle (os_lock_t) from os.h
 *  - No direct FreeRTOS/CMSIS-RTOS types appear in this header
 *  - Unlike SPI, a device address is passed per call (I2C is a shared,
 *    addressed bus - there is no per-device chip-select line)
 *  - Locking policy: no i2c_io_* function below acquires me->lock itself
 *    (same policy as spi_io.h). The application/task layer calls
 *    i2c_io_lock() before a sequence of one or more i2c_io_* calls that
 *    must execute as a single bus transaction, and i2c_io_unlock() after.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "os.h"
#include "define.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#ifndef I2C_WAIT_TIME
#define I2C_WAIT_TIME       (250u)      /* default timeout in ms (lock + per-byte waits) */
#endif

#ifndef I2C_DEFAULT_SPEED_HZ
#define I2C_DEFAULT_SPEED_HZ (100000u)  /* Standard-mode default; see i2c_io_set_speed() for Fast-mode (<=400kHz) */
#endif

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/**
 * @struct i2c_io_t
 * @brief I2C bus context (port + OS lock).
 *
 * ui32I2cPort: user-defined I2C instance index/ID (e.g., 1 for I2C1).
 * lock: protects exclusive access to the I2C bus from multiple tasks.
 */
typedef struct
{
    uint32_t   ui32I2cPort;
    os_lock_t  lock;
} i2c_io_t;

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

/* Configuration */
uint32_t i2c_io_init(i2c_io_t *me);
uint32_t i2c_io_set_speed(i2c_io_t *me, uint32_t ui32SpeedHz);

/*
 * Shared helpers (implemented once, in i2c_io.c; not re-implemented per
 * platform backend):
 *   - i2c_io_lock_init(): initializes me->lock. Every backend's
 *     i2c_io_init() must call this before touching any hardware.
 *   - i2c_io_lock()/i2c_io_unlock(): the application/task layer calls
 *     these around the i2c_io_* calls that make up one bus transaction
 *     (see the locking-policy note at the top of this file).
 */
uint32_t i2c_io_lock_init(i2c_io_t *me);
uint32_t i2c_io_lock(i2c_io_t *me, uint32_t timeout_ms);
uint32_t i2c_io_unlock(i2c_io_t *me);

/* Synchronous (blocking) */
uint32_t i2c_io_read_sync(i2c_io_t *me, uint16_t ui16DevAddr,
                          uint8_t *pui8RxBuff, uint32_t ui32Length);
uint32_t i2c_io_write_sync(i2c_io_t *me, uint16_t ui16DevAddr,
                           const uint8_t *pui8TxBuff, uint32_t ui32Length);
/* Write ui32TxLength bytes, then (repeated START, no STOP in between) read
 * ui32RxLength bytes - the standard "set register pointer, then read it"
 * pattern used by e.g. the 24C256 EEPROM. */
uint32_t i2c_io_write_read_sync(i2c_io_t *me, uint16_t ui16DevAddr,
                                const uint8_t *pui8TxBuff, uint32_t ui32TxLength,
                                uint8_t *pui8RxBuff, uint32_t ui32RxLength);

/* Asynchronous (interrupt/callback driven) - declared for future backends,
 * not implemented by the stm32/ backend yet. */
uint32_t i2c_io_read_async(i2c_io_t *me, uint16_t ui16DevAddr,
                           uint8_t *pui8RxBuff, uint32_t ui32Length);
uint32_t i2c_io_write_async(i2c_io_t *me, uint16_t ui16DevAddr,
                            const uint8_t *pui8TxBuff, uint32_t ui32Length);

/* DMA - declared for future backends, not implemented by the stm32/
 * backend yet. */
uint32_t i2c_io_read_dma(i2c_io_t *me, uint16_t ui16DevAddr,
                         uint8_t *pui8RxBuff, uint32_t ui32Length);
uint32_t i2c_io_write_dma(i2c_io_t *me, uint16_t ui16DevAddr,
                          const uint8_t *pui8TxBuff, uint32_t ui32Length);
