/*
 * bsp_eeprom.c
 */

#include "bsp_eeprom.h"
#include "eeprom24cxx.h"
#include "bsp_i2c1.h"

static eeprom24cxx_t s_eeprom;

void bsp_eeprom_init(void)
{
    bsp_i2c1_init(); /* idempotent - see bsp_i2c1.h */

    eeprom24cxx_init_t init = {
        .pI2cBus = bsp_i2c1_get(),
        .ui16DevAddr = EEPROM24C256_I2C_ADDR,
        .ui32PageSizeBytes = EEPROM24C256_PAGE_SIZE_BYTES,
        .ui32SizeBytes = EEPROM24C256_SIZE_BYTES,
    };

    eeprom24cxx_init(&s_eeprom, &init);
}

uint32_t bsp_eeprom_write(uint16_t ui16Addr, const uint8_t *pui8Buff, uint32_t ui32Length)
{
    return eeprom24cxx_write(&s_eeprom, ui16Addr, pui8Buff, ui32Length);
}

uint32_t bsp_eeprom_read(uint16_t ui16Addr, uint8_t *pui8Buff, uint32_t ui32Length)
{
    return eeprom24cxx_read(&s_eeprom, ui16Addr, pui8Buff, ui32Length);
}
