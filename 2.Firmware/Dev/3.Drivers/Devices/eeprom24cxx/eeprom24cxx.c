/*
 * eeprom24cxx.c
 */

#include <stddef.h>

#include "eeprom24cxx.h"
#include "i2c_io.h"
#include "define.h"

/*
 * ACK-polling: the chip NACKs the address phase of any transaction while
 * the internal write cycle (tWR) is still in progress, and ACKs again as
 * soon as it's done - cheaper and faster than a fixed ~5ms busy-wait.
 * Caller must already hold me->pI2cBus's lock.
 */
static uint32_t eeprom24cxx_wait_write_done(eeprom24cxx_t *me)
{
    uint32_t start_tick = os_tick_get();
    uint8_t poll_byte = 0U;

    for (;;) {
        uint32_t err = i2c_io_write_sync(me->pI2cBus, me->ui16DevAddr, &poll_byte, 1U);

        if (err == ERROR_OK) {
            return ERROR_OK;
        }

        if (err != ERROR_NACK) {
            return err;
        }

        if ((os_tick_get() - start_tick) >= EEPROM24CXX_WRITE_TIMEOUT_MS) {
            return ERROR_TIMEOUT;
        }
    }
}

uint32_t eeprom24cxx_init(eeprom24cxx_t *me, const eeprom24cxx_init_t *init)
{
    if ((me == NULL) || (init == NULL) || (init->pI2cBus == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    me->pI2cBus = init->pI2cBus;
    me->ui16DevAddr = init->ui16DevAddr;
    me->ui32PageSizeBytes = (init->ui32PageSizeBytes != 0U) ?
                             init->ui32PageSizeBytes : EEPROM24C256_PAGE_SIZE_BYTES;
    me->ui32SizeBytes = (init->ui32SizeBytes != 0U) ?
                         init->ui32SizeBytes : EEPROM24C256_SIZE_BYTES;

    return ERROR_OK;
}

uint32_t eeprom24cxx_write_byte(eeprom24cxx_t *me, uint16_t ui16Addr, uint8_t ui8Data)
{
    return eeprom24cxx_write(me, ui16Addr, &ui8Data, 1U);
}

uint32_t eeprom24cxx_read_byte(eeprom24cxx_t *me, uint16_t ui16Addr, uint8_t *pui8Data)
{
    return eeprom24cxx_read(me, ui16Addr, pui8Data, 1U);
}

uint32_t eeprom24cxx_write(eeprom24cxx_t *me, uint16_t ui16Addr,
                           const uint8_t *pui8Buff, uint32_t ui32Length)
{
    if ((me == NULL) || (pui8Buff == NULL) || (ui32Length == 0U)) {
        return ERROR_INVALID_PARAM;
    }

    if (((uint32_t)ui16Addr + ui32Length) > me->ui32SizeBytes) {
        return ERROR_INVALID_PARAM;
    }

    uint32_t err = i2c_io_lock(me->pI2cBus, I2C_WAIT_TIME);

    if (err != ERROR_OK) {
        return err;
    }

    uint32_t offset = 0U;

    while (offset < ui32Length) {
        uint16_t addr = ui16Addr + offset;
        uint32_t bytes_to_page_end = me->ui32PageSizeBytes - (addr % me->ui32PageSizeBytes);
        uint32_t remaining = ui32Length - offset;
        uint32_t chunk = (remaining < bytes_to_page_end) ? remaining : bytes_to_page_end;

        /* [addrHi, addrLo, data...] in one SPI-style framed transfer, same
         * pattern as i2c_io_write_sync's other callers. */
        uint8_t frame[2U + EEPROM24C256_PAGE_SIZE_BYTES];

        frame[0] = (uint8_t)(addr >> 8);
        frame[1] = (uint8_t)(addr & 0xFFU);

        for (uint32_t i = 0U; i < chunk; i++) {
            frame[2U + i] = pui8Buff[offset + i];
        }

        err = i2c_io_write_sync(me->pI2cBus, me->ui16DevAddr, frame, chunk + 2U);

        if (err == ERROR_OK) {
            err = eeprom24cxx_wait_write_done(me);
        }

        if (err != ERROR_OK) {
            break;
        }

        offset += chunk;
    }

    i2c_io_unlock(me->pI2cBus);

    return err;
}

uint32_t eeprom24cxx_read(eeprom24cxx_t *me, uint16_t ui16Addr,
                          uint8_t *pui8Buff, uint32_t ui32Length)
{
    if ((me == NULL) || (pui8Buff == NULL) || (ui32Length == 0U)) {
        return ERROR_INVALID_PARAM;
    }

    if (((uint32_t)ui16Addr + ui32Length) > me->ui32SizeBytes) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t addr_buf[2] = { (uint8_t)(ui16Addr >> 8), (uint8_t)(ui16Addr & 0xFFU) };

    uint32_t err = i2c_io_lock(me->pI2cBus, I2C_WAIT_TIME);

    if (err != ERROR_OK) {
        return err;
    }

    /* Set the address pointer, then (repeated START, no STOP) read -
     * exactly the i2c_io_write_read_sync use case documented in i2c_io.h. */
    err = i2c_io_write_read_sync(me->pI2cBus, me->ui16DevAddr,
                                  addr_buf, 2U, pui8Buff, ui32Length);

    i2c_io_unlock(me->pI2cBus);

    return err;
}
