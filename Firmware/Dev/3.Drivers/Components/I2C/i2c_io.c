/*
 * i2c_io.c
 *
 * Shared across every platform backend (Dev/3.Drivers/I2C/<platform>/):
 * only the OS-level lock setup is truly platform-agnostic, so each
 * backend's own i2c_io_init() calls this first, then goes on to do its
 * hardware-specific setup (clocks, GPIO, peripheral registers).
 */

#include <stddef.h>

#include "i2c_io.h"
#include "os.h"
#include "define.h"

uint32_t i2c_io_lock_init(i2c_io_t *me)
{
	if (me == NULL) {
		return ERROR_INVALID_PARAM;
	}

	return os_lock_init(&me->lock) ? ERROR_OK : ERROR_FAIL;
}

/*
 * Application/task layer calls these around a sequence of i2c_io_*
 * calls that must execute as one bus transaction - see the locking
 * policy note in i2c_io.h. The driver itself never calls these.
 */
uint32_t i2c_io_lock(i2c_io_t *me, uint32_t timeout_ms)
{
	if (me == NULL) {
		return ERROR_INVALID_PARAM;
	}

	return os_lock_acquire(&me->lock, timeout_ms);
}

uint32_t i2c_io_unlock(i2c_io_t *me)
{
	if (me == NULL) {
		return ERROR_INVALID_PARAM;
	}

	return os_lock_release(&me->lock);
}
