/*
 * eeprom24cxx.h
 *
 * Driver for I2C EEPROM chips in the 24Cxx family (24C256 in this
 * system's design - see 1.Document/docs/hardware/connections.md #4),
 * built on the platform-agnostic i2c_io.h bus abstraction so it works
 * unchanged on top of any i2c_io backend.
 *
 * 24Cxx addressing: the chip uses a 2-byte (16-bit, big-endian on the
 * wire) memory address regardless of capacity - 24C256 is 32768 bytes
 * (0x0000-0x7FFF), fully covered by 16 bits. Writes are paged (64 bytes
 * per page on the 24C256): a write that would cross a page boundary is
 * split into multiple page writes by eeprom24cxx_write(), each followed
 * by the write-cycle wait (tWR, ~5ms) via ACK polling.
 */

#pragma once

#include <stdint.h>

#include "i2c_io.h"
#include "define.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#ifndef EEPROM24CXX_WRITE_TIMEOUT_MS
#define EEPROM24CXX_WRITE_TIMEOUT_MS    (10u)   /* tWR is ~5ms; leaves margin */
#endif

/* 24C256 defaults - pass different values in eeprom24cxx_init_t for other
 * 24Cxx parts (e.g. 24C32/24C64 have smaller pages/capacity). */
#define EEPROM24C256_PAGE_SIZE_BYTES    (64u)
#define EEPROM24C256_SIZE_BYTES         (32768u)
#define EEPROM24C256_I2C_ADDR           (0x50u)

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

typedef struct
{
    i2c_io_t *pI2cBus;
    uint16_t  ui16DevAddr;      /* 7-bit I2C address, e.g. 0x50 */
    uint32_t  ui32PageSizeBytes;
    uint32_t  ui32SizeBytes;
} eeprom24cxx_init_t;

typedef struct
{
    i2c_io_t *pI2cBus;
    uint16_t  ui16DevAddr;
    uint32_t  ui32PageSizeBytes;
    uint32_t  ui32SizeBytes;
} eeprom24cxx_t;

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

uint32_t eeprom24cxx_init(eeprom24cxx_t *me, const eeprom24cxx_init_t *init);

uint32_t eeprom24cxx_write_byte(eeprom24cxx_t *me, uint16_t ui16Addr, uint8_t ui8Data);
uint32_t eeprom24cxx_read_byte(eeprom24cxx_t *me, uint16_t ui16Addr, uint8_t *pui8Data);

/* Handles page-boundary splitting and the write-cycle wait internally -
 * safe to call with a buffer that spans multiple pages. */
uint32_t eeprom24cxx_write(eeprom24cxx_t *me, uint16_t ui16Addr,
                           const uint8_t *pui8Buff, uint32_t ui32Length);
uint32_t eeprom24cxx_read(eeprom24cxx_t *me, uint16_t ui16Addr,
                          uint8_t *pui8Buff, uint32_t ui32Length);
