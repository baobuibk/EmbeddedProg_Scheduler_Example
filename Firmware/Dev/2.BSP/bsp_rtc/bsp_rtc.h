/*
 * bsp_rtc.h
 *
 * Wires the DS1307 driver (Dev/3.Drivers/Devices/ds1307) to this
 * board's shared I2C1 bus (bsp_i2c1). Thin wrapper only - no caching,
 * every call here does a real I2C transaction. See
 * Dev/1.Application/app_rtc for the periodic-read + change-notification
 * layer built on top of this (what LCD/log consumers should actually
 * use instead of calling this directly).
 */

#pragma once

#include <stdint.h>

#include "ds1307.h"

void bsp_rtc_init(void);

uint32_t bsp_rtc_get_time(ds1307_time_t *pTime);
uint32_t bsp_rtc_set_time(const ds1307_time_t *pTime);
