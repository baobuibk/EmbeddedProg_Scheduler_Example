/*
 * bsp_eeprom.h
 *
 * Wires the 24C256 driver (Dev/3.Drivers/Devices/eeprom24cxx) to this
 * board's shared I2C1 bus (bsp_i2c1), addr 0x50 - see
 * 1.Document/docs/hardware/connections.md #4. Thin wrapper only - no
 * caching, every call here does a real I2C transaction (same shape as
 * bsp_rtc, the other I2C1 device already wired up).
 */

#pragma once

#include <stdint.h>

void bsp_eeprom_init(void);

uint32_t bsp_eeprom_write(uint16_t ui16Addr, const uint8_t *pui8Buff, uint32_t ui32Length);
uint32_t bsp_eeprom_read(uint16_t ui16Addr, uint8_t *pui8Buff, uint32_t ui32Length);
