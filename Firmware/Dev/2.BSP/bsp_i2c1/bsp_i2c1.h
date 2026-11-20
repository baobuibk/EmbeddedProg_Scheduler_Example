/*
 * bsp_i2c1.h
 *
 * Owns the single i2c_io_t instance for this board's I2C1 bus (PB6=SCL,
 * PB7=SDA - see 1.Document/docs/hardware/connections.md), shared by
 * every I2C1 device on this board (DS1307 RTC today via bsp_rtc; the
 * EEPROM and LCD backpack will share it too once their own BSP modules
 * exist).
 *
 * This matters for correctness, not just tidiness: i2c_io_t carries its
 * own lock (see i2c_io.h's locking policy), so two separate i2c_io_t
 * instances both pointed at port 1 would NOT actually exclude each
 * other on the real, physical bus - only one shared instance does.
 */

#pragma once

#include "i2c_io.h"

/* Idempotent - safe to call from more than one BSP module's init (each
 * I2C1 device's BSP should call this itself rather than assume another
 * module already did), it just re-applies the same bus config. */
void bsp_i2c1_init(void);

/* Never NULL after bsp_i2c1_init() has run at least once. */
i2c_io_t *bsp_i2c1_get(void);
