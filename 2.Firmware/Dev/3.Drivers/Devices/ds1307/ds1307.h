/*
 * ds1307.h
 *
 * Driver for the DS1307 I2C RTC (see
 * 1.Document/docs/hardware/connections.md #5 - fixed address 0x68,
 * shared I2C1 bus with the EEPROM and LCD backpack), built on the
 * platform-agnostic i2c_io.h bus abstraction.
 *
 * Only the time-keeping registers (0x00-0x06) and the oscillator-enable
 * (CH) bit are exposed - this design doesn't wire SQW/OUT (see the
 * hardware doc), so the control register (0x07, square-wave/interrupt
 * output config) is intentionally not part of this driver's API.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "i2c_io.h"
#include "define.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define DS1307_I2C_ADDR         (0x68u)

#define DS1307_REG_SECONDS      (0x00u)   /* bit7 = CH (clock halt, 1=stopped) */
#define DS1307_REG_MINUTES      (0x01u)
#define DS1307_REG_HOURS        (0x02u)   /* bit6 = 12/24h select (1=12h) */
#define DS1307_REG_DAY          (0x03u)   /* 1-7, weekday (no fixed meaning) */
#define DS1307_REG_DATE         (0x04u)   /* 1-31 */
#define DS1307_REG_MONTH        (0x05u)   /* 1-12 */
#define DS1307_REG_YEAR         (0x06u)   /* 0-99, offset from 2000 */

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

typedef struct
{
    uint8_t  ui8Seconds;    /* 0-59 */
    uint8_t  ui8Minutes;    /* 0-59 */
    uint8_t  ui8Hours;      /* 0-23 */
    uint8_t  ui8Day;        /* 1-7 */
    uint8_t  ui8Date;       /* 1-31 */
    uint8_t  ui8Month;      /* 1-12 */
    uint16_t ui16Year;      /* e.g. 2025 */
} ds1307_time_t;

typedef struct
{
    i2c_io_t *pI2cBus;
} ds1307_init_t;

typedef struct
{
    i2c_io_t *pI2cBus;
} ds1307_t;

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

/* Clears CH (starts the oscillator) if it finds it set - the usual
 * "first power-up / battery was fully dead" state - leaves the running
 * clock alone otherwise. */
uint32_t ds1307_init(ds1307_t *me, const ds1307_init_t *init);

uint32_t ds1307_write_register(ds1307_t *me, uint8_t ui8Reg, uint8_t ui8Value);
uint32_t ds1307_read_register(ds1307_t *me, uint8_t ui8Reg, uint8_t *pui8Value);

uint32_t ds1307_set_time(ds1307_t *me, const ds1307_time_t *time);
uint32_t ds1307_get_time(ds1307_t *me, ds1307_time_t *time);

/* ui8Running: true if the oscillator is enabled (CH bit clear). */
uint32_t ds1307_is_running(ds1307_t *me, bool *pbRunning);
uint32_t ds1307_start(ds1307_t *me);  /* clear CH */
uint32_t ds1307_stop(ds1307_t *me);   /* set CH */
