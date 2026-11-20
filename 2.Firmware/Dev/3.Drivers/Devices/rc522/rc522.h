/*
 * rc522.h
 *
 * Driver for the MFRC522 RFID reader module (see
 * 1.Document/docs/hardware/connections.md #6 - SPI1 + software CS on
 * PA4 + RST on PB0), built on the platform-agnostic spi_io.h/do.h
 * abstractions, same style as Dev/3.Drivers/Devices/ds3234 (SPI device,
 * CS toggled by the caller's do_t around a locked transfer).
 *
 * Scope: card presence (REQA) and single-size (4-byte) UID read
 * (anticollision cascade level 1 only) - covers the Mifare
 * Classic/Ultralight cards used by this system's UC-02 (see
 * 1.Document/docs/use-cases.md). 7/10-byte UIDs (cascade levels 2/3,
 * "double/triple size" cards) are not handled.
 *
 * This driver does not use the IRQ pin (PB5, EXTI) - it's a polled
 * (ComIrqReg) implementation; wiring IRQ-driven wake-up is an
 * application/scheduler-layer concern, not this driver's.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "spi_io.h"
#include "do.h"
#include "define.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#ifndef RC522_TIMEOUT_MS
#define RC522_TIMEOUT_MS    (50u)
#endif

#define RC522_UID_LEN        (4u)   /* cascade level 1 only - see header note */

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

typedef struct
{
    spi_io_t *pSpiBus;
    do_t     *pCsPin;
    do_t     *pRstPin;
} rc522_init_t;

typedef struct
{
    spi_io_t *pSpiBus;
    do_t     *pCsPin;
    do_t     *pRstPin;
} rc522_t;

/******************************************************************************/
/*************************** Public API Declarations **************************/
/******************************************************************************/

/* Hardware-resets the module (pRstPin), then runs PCD_Init: soft reset,
 * timer/ASK/mode setup, antenna on. */
uint32_t rc522_init(rc522_t *me, const rc522_init_t *init);

uint32_t rc522_read_register(rc522_t *me, uint8_t ui8Reg, uint8_t *pui8Value);
uint32_t rc522_write_register(rc522_t *me, uint8_t ui8Reg, uint8_t ui8Value);

uint32_t rc522_antenna_on(rc522_t *me);
uint32_t rc522_antenna_off(rc522_t *me);

/* ERROR_OK if a PICC answered REQA (card present in the field),
 * ERROR_TIMEOUT if none did. */
uint32_t rc522_request_a(rc522_t *me);

/* Runs REQA then cascade-level-1 anticollision; fills pui8Uid[4] (and,
 * if non-NULL, *pui8Sak with the SELECT response's SAK byte) on
 * ERROR_OK. ERROR_TIMEOUT means no card in the field; ERROR_FAIL means
 * a card answered REQA but the UID read failed a BCC/CRC check. */
uint32_t rc522_read_uid(rc522_t *me, uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak);

/* Puts a selected PICC back into HALT (idle) state. */
uint32_t rc522_halt(rc522_t *me);
