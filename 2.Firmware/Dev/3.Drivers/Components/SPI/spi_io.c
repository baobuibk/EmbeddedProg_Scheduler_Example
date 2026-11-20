/*
 * spi_io.c
 *
 *  Created on: Oct 11, 2025
 *      Author: Admin
 */

#include <stddef.h>

#include "spi_io.h"
#include "os.h"
#include "define.h"

/*
 * Shared across every platform backend (Dev/3.Drivers/SPI/<platform>/):
 * only the OS-level lock setup is truly platform-agnostic, so each
 * backend's own spi_io_init() calls this first, then goes on to do its
 * hardware-specific setup (clocks, GPIO, peripheral registers).
 */
uint32_t spi_io_lock_init(spi_io_t *me)
{
	if (me == NULL) {
		return ERROR_INVALID_PARAM;
	}

	return os_lock_init(&me->lock) ? ERROR_OK : ERROR_FAIL;
}

/*
 * Application/task layer calls these around a sequence of spi_io_*
 * calls that must execute as one bus transaction - see the locking
 * policy note in spi_io.h. The driver itself never calls these.
 */
uint32_t spi_io_lock(spi_io_t *me, uint32_t timeout_ms)
{
	if (me == NULL) {
		return ERROR_INVALID_PARAM;
	}

	return os_lock_acquire(&me->lock, timeout_ms);
}

uint32_t spi_io_unlock(spi_io_t *me)
{
	if (me == NULL) {
		return ERROR_INVALID_PARAM;
	}

	return os_lock_release(&me->lock);
}

