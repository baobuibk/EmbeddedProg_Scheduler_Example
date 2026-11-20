/*
 * bsp_rc522.h
 *
 * Wires the MFRC522 driver (Dev/3.Drivers/Devices/rc522) to this
 * board's SPI2 bus + CS/RST pins (SCK=PB13, MISO=PB14, MOSI=PB15,
 * CS=PA15, RST=PB0 - see 1.Document/docs/hardware/connections.md #6).
 * CS was moved off PB12 (its first assignment) purely to dodge a
 * Proteus-simulation-only bug: PB12 doubles as TIM1_BKIN by default,
 * and that specific VSM Cortex-M3 model falsely reports "logic
 * contention" and crashes (access violation) on that pin when bit-
 * banged as a CS line, even though TIM1 is never clocked by this
 * firmware (no real contention exists on actual silicon). PA15 is used
 * as plain GPIO here, not as JTDI - fine for SWD-only debugging, which
 * never drives PA15/PB3/PB4 - see connections.md's pinout note.
 * SPI2 has no other consumer on this board, so the spi_io_t instance is
 * owned directly here rather than split into a separate bsp_spi2
 * singleton (compare Dev/2.BSP/bsp_i2c1, needed there because I2C1 has
 * several devices) - if a second SPI2 device ever shows up, pull that
 * part out the same way bsp_i2c1 was.
 *
 * Purely polled, on purpose: this board's scheduler is cooperative and
 * app_rc522_task() already blocks synchronously on bsp_rc522_poll()'s
 * SPI wait, so there is no point left in it for the IRQ pin (PB5) to
 * buy - no other task can run "while waiting" either way, so an
 * EXTI-driven event would fire essentially back-to-back with the poll
 * call returning, not actually overlapping anything. See
 * Dev/1.Application/app_rc522 for where the UID is actually read (right
 * inside the same poll task, immediately after a successful REQA). The
 * IRQ pin stays wired on the board for whenever rc522.c's transceive
 * path becomes genuinely non-blocking/async - only then would routing
 * it through Dev/4.MiddleWare/event buy anything real.
 */

#pragma once

#include <stdint.h>

#include "rc522.h"

void bsp_rc522_init(void);

/* Cheap "ask if anyone is there" - REQA only, no anticollision. Blocks
 * the caller for up to RC522_TIMEOUT_MS (rc522.h, default 50ms) when no
 * card answers - acceptable for this lab's cooperative scheduler/task
 * cadence, but worth knowing before lowering app_rc522_task's period
 * much below that. */
uint32_t bsp_rc522_poll(void);

/* Full anticollision + SELECT UID read - heavier than bsp_rc522_poll(),
 * only worth calling after it returned ERROR_OK. */
uint32_t bsp_rc522_read_uid(uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak);

uint32_t bsp_rc522_halt(void);
