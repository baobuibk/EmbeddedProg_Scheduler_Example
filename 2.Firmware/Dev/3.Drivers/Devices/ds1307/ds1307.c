/*
 * ds1307.c
 */

#include <stddef.h>
#include <stdbool.h>

#include "ds1307.h"
#include "i2c_io.h"
#include "define.h"

#define DS1307_CH_BIT_MASK      (0x80u)
#define DS1307_HOUR_12H_MASK    (0x40u)

static uint8_t ds1307_dec_to_bcd(uint8_t dec)
{
    return (uint8_t)(((dec / 10U) << 4U) | (dec % 10U));
}

static uint8_t ds1307_bcd_to_dec(uint8_t bcd)
{
    return (uint8_t)(((bcd >> 4U) * 10U) + (bcd & 0x0FU));
}

uint32_t ds1307_write_register(ds1307_t *me, uint8_t ui8Reg, uint8_t ui8Value)
{
    if (me == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t frame[2] = { ui8Reg, ui8Value };

    uint32_t err = i2c_io_lock(me->pI2cBus, I2C_WAIT_TIME);

    if (err != ERROR_OK) {
        return err;
    }

    err = i2c_io_write_sync(me->pI2cBus, DS1307_I2C_ADDR, frame, 2U);

    i2c_io_unlock(me->pI2cBus);

    return err;
}

uint32_t ds1307_read_register(ds1307_t *me, uint8_t ui8Reg, uint8_t *pui8Value)
{
    if ((me == NULL) || (pui8Value == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    uint32_t err = i2c_io_lock(me->pI2cBus, I2C_WAIT_TIME);

    if (err != ERROR_OK) {
        return err;
    }

    err = i2c_io_write_read_sync(me->pI2cBus, DS1307_I2C_ADDR, &ui8Reg, 1U, pui8Value, 1U);

    i2c_io_unlock(me->pI2cBus);

    return err;
}

uint32_t ds1307_init(ds1307_t *me, const ds1307_init_t *init)
{
    if ((me == NULL) || (init == NULL) || (init->pI2cBus == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    me->pI2cBus = init->pI2cBus;

    uint8_t seconds_reg;
    uint32_t err = ds1307_read_register(me, DS1307_REG_SECONDS, &seconds_reg);

    if (err != ERROR_OK) {
        return err;
    }

    if ((seconds_reg & DS1307_CH_BIT_MASK) != 0U) {
        /* Oscillator was halted (fresh chip / battery was fully dead) -
         * clear CH so the clock actually starts counting. */
        err = ds1307_write_register(me, DS1307_REG_SECONDS, seconds_reg & ~DS1307_CH_BIT_MASK);
    }

    return err;
}

uint32_t ds1307_set_time(ds1307_t *me, const ds1307_time_t *time)
{
    if ((me == NULL) || (time == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    /* Hours written with bit6=0 (24h mode) and CH left clear (bit7=0 on
     * the seconds byte), matching ds1307_init()'s oscillator-enabled
     * state - this driver only ever runs the chip in 24h mode. */
    uint32_t err = ds1307_write_register(me, DS1307_REG_SECONDS, ds1307_dec_to_bcd(time->ui8Seconds));

    if (err == ERROR_OK) {
        err = ds1307_write_register(me, DS1307_REG_MINUTES, ds1307_dec_to_bcd(time->ui8Minutes));
    }

    if (err == ERROR_OK) {
        err = ds1307_write_register(me, DS1307_REG_HOURS, ds1307_dec_to_bcd(time->ui8Hours) & ~DS1307_HOUR_12H_MASK);
    }

    if (err == ERROR_OK) {
        err = ds1307_write_register(me, DS1307_REG_DAY, ds1307_dec_to_bcd(time->ui8Day));
    }

    if (err == ERROR_OK) {
        err = ds1307_write_register(me, DS1307_REG_DATE, ds1307_dec_to_bcd(time->ui8Date));
    }

    if (err == ERROR_OK) {
        err = ds1307_write_register(me, DS1307_REG_MONTH, ds1307_dec_to_bcd(time->ui8Month));
    }

    if (err == ERROR_OK) {
        err = ds1307_write_register(me, DS1307_REG_YEAR, ds1307_dec_to_bcd((uint8_t)(time->ui16Year % 100U)));
    }

    return err;
}

uint32_t ds1307_get_time(ds1307_t *me, ds1307_time_t *time)
{
    if ((me == NULL) || (time == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t seconds_reg, minutes_reg, hours_reg, day_reg, date_reg, month_reg, year_reg;

    uint32_t err = ds1307_read_register(me, DS1307_REG_SECONDS, &seconds_reg);

    if (err == ERROR_OK) {
        err = ds1307_read_register(me, DS1307_REG_MINUTES, &minutes_reg);
    }

    if (err == ERROR_OK) {
        err = ds1307_read_register(me, DS1307_REG_HOURS, &hours_reg);
    }

    if (err == ERROR_OK) {
        err = ds1307_read_register(me, DS1307_REG_DAY, &day_reg);
    }

    if (err == ERROR_OK) {
        err = ds1307_read_register(me, DS1307_REG_DATE, &date_reg);
    }

    if (err == ERROR_OK) {
        err = ds1307_read_register(me, DS1307_REG_MONTH, &month_reg);
    }

    if (err == ERROR_OK) {
        err = ds1307_read_register(me, DS1307_REG_YEAR, &year_reg);
    }

    if (err != ERROR_OK) {
        return err;
    }

    time->ui8Seconds = ds1307_bcd_to_dec(seconds_reg & ~DS1307_CH_BIT_MASK);
    time->ui8Minutes = ds1307_bcd_to_dec(minutes_reg);
    /* 24h mode only (see ds1307_set_time()); mask off the mode-select bit
     * in case the chip was left in 12h mode by something else. */
    time->ui8Hours = ds1307_bcd_to_dec(hours_reg & ~DS1307_HOUR_12H_MASK & 0x3FU);
    time->ui8Day = ds1307_bcd_to_dec(day_reg);
    time->ui8Date = ds1307_bcd_to_dec(date_reg);
    time->ui8Month = ds1307_bcd_to_dec(month_reg);
    time->ui16Year = 2000U + ds1307_bcd_to_dec(year_reg);

    return ERROR_OK;
}

uint32_t ds1307_is_running(ds1307_t *me, bool *pbRunning)
{
    if ((me == NULL) || (pbRunning == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t seconds_reg;
    uint32_t err = ds1307_read_register(me, DS1307_REG_SECONDS, &seconds_reg);

    if (err == ERROR_OK) {
        *pbRunning = ((seconds_reg & DS1307_CH_BIT_MASK) == 0U);
    }

    return err;
}

uint32_t ds1307_start(ds1307_t *me)
{
    uint8_t seconds_reg;
    uint32_t err = ds1307_read_register(me, DS1307_REG_SECONDS, &seconds_reg);

    if (err != ERROR_OK) {
        return err;
    }

    return ds1307_write_register(me, DS1307_REG_SECONDS, seconds_reg & ~DS1307_CH_BIT_MASK);
}

uint32_t ds1307_stop(ds1307_t *me)
{
    uint8_t seconds_reg;
    uint32_t err = ds1307_read_register(me, DS1307_REG_SECONDS, &seconds_reg);

    if (err != ERROR_OK) {
        return err;
    }

    return ds1307_write_register(me, DS1307_REG_SECONDS, seconds_reg | DS1307_CH_BIT_MASK);
}
