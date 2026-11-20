/*
 * define.h
 *
 * Common error codes shared by all Dev/ drivers and middleware, so a
 * platform implementation (e.g. Dev/3.Drivers/SPI/stm32/) and the code
 * that calls it agree on return values without depending on each other's
 * headers.
 */

#pragma once

#define ERROR_OK                (0u)
#define ERROR_FAIL              (1u)
#define ERROR_INVALID_PARAM     (2u)
#define ERROR_TIMEOUT           (3u)
#define ERROR_BUSY              (4u)
#define ERROR_SPI               (5u)
#define ERROR_NACK              (6u)    /* I2C: address or data byte not acknowledged */
