/*
 * rc522.c
 */

#include <stddef.h>
#include <stdbool.h>

#include "rc522.h"
#include "spi_io.h"
#include "do.h"
#include "os.h"
#include "define.h"

/*==============================================================================
 * MFRC522 register map (subset used by this driver)
 *============================================================================*/

#define REG_COMMAND         (0x01u)
#define REG_COM_IRQ         (0x04u)
#define REG_DIV_IRQ         (0x05u)
#define REG_ERROR           (0x06u)
#define REG_STATUS2         (0x08u)
#define REG_FIFO_DATA       (0x09u)
#define REG_FIFO_LEVEL      (0x0Au)
#define REG_CONTROL         (0x0Cu)
#define REG_BIT_FRAMING     (0x0Du)
#define REG_COLL            (0x0Eu)
#define REG_MODE            (0x11u)
#define REG_TX_CONTROL      (0x14u)
#define REG_TX_ASK          (0x15u)
#define REG_T_MODE          (0x2Au)
#define REG_T_PRESCALER     (0x2Bu)
#define REG_T_RELOAD_H      (0x2Cu)
#define REG_T_RELOAD_L      (0x2Du)
#define REG_VERSION         (0x37u)

/* PCD (reader-chip) commands, written to REG_COMMAND */
#define PCD_CMD_IDLE            (0x00u)
#define PCD_CMD_TRANSCEIVE      (0x0Cu)
#define PCD_CMD_SOFT_RESET      (0x0Fu)

/* PICC (card) commands, sent over the air via FIFO */
#define PICC_CMD_REQA           (0x26u)
#define PICC_CMD_ANTICOLL_CL1   (0x93u)
#define PICC_CMD_HALTA          (0x50u)

#define COM_IRQ_TIMER_IRQ_MASK  (0x01u)
#define COM_IRQ_RX_IRQ_MASK     (0x20u)
#define ERROR_REG_COLL_MASK     (0x08u)
#define TX_CONTROL_TX2RF_MASK  (0x02u)
#define TX_CONTROL_TX1RF_MASK  (0x01u)

/*==============================================================================
 * Low-level register access
 *
 * One SPI transfer per register access, CS held low for its duration -
 * same CS-around-a-locked-transfer pattern as
 * Dev/3.Drivers/Devices/ds3234. Address byte layout (datasheet 8.1.2.1):
 * bit7=1 for read / 0 for write, bits6-1=register address, bit0=0.
 *============================================================================*/

static uint8_t rc522_addr_byte(uint8_t ui8Reg, bool bRead)
{
    return (uint8_t)(((ui8Reg << 1U) & 0x7EU) | (bRead ? 0x80U : 0x00U));
}

uint32_t rc522_write_register(rc522_t *me, uint8_t ui8Reg, uint8_t ui8Value)
{
    if (me == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t frame[2] = { rc522_addr_byte(ui8Reg, false), ui8Value };

    uint32_t err = spi_io_lock(me->pSpiBus, SPI_WAIT_TIME);

    if (err != ERROR_OK) {
        return err;
    }

    do_reset(me->pCsPin);
    err = spi_io_write_sync(me->pSpiBus, frame, 2U);
    do_set(me->pCsPin);

    spi_io_unlock(me->pSpiBus);

    return err;
}

uint32_t rc522_read_register(rc522_t *me, uint8_t ui8Reg, uint8_t *pui8Value)
{
    if ((me == NULL) || (pui8Value == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t tx[2] = { rc522_addr_byte(ui8Reg, true), 0x00U };
    uint8_t rx[2];

    uint32_t err = spi_io_lock(me->pSpiBus, SPI_WAIT_TIME);

    if (err != ERROR_OK) {
        return err;
    }

    do_reset(me->pCsPin);
    err = spi_io_transfer_sync(me->pSpiBus, tx, rx, 2U);
    do_set(me->pCsPin);

    spi_io_unlock(me->pSpiBus);

    *pui8Value = rx[1];

    return err;
}

static uint32_t rc522_set_register_bitmask(rc522_t *me, uint8_t ui8Reg, uint8_t ui8Mask)
{
    uint8_t value;
    uint32_t err = rc522_read_register(me, ui8Reg, &value);

    if (err != ERROR_OK) {
        return err;
    }

    return rc522_write_register(me, ui8Reg, value | ui8Mask);
}

static uint32_t rc522_clear_register_bitmask(rc522_t *me, uint8_t ui8Reg, uint8_t ui8Mask)
{
    uint8_t value;
    uint32_t err = rc522_read_register(me, ui8Reg, &value);

    if (err != ERROR_OK) {
        return err;
    }

    return rc522_write_register(me, ui8Reg, value & ~ui8Mask);
}

/*==============================================================================
 * Configuration
 *============================================================================*/

uint32_t rc522_antenna_on(rc522_t *me)
{
    return rc522_set_register_bitmask(me, REG_TX_CONTROL,
                                       TX_CONTROL_TX1RF_MASK | TX_CONTROL_TX2RF_MASK);
}

uint32_t rc522_antenna_off(rc522_t *me)
{
    return rc522_clear_register_bitmask(me, REG_TX_CONTROL,
                                         TX_CONTROL_TX1RF_MASK | TX_CONTROL_TX2RF_MASK);
}

static uint32_t rc522_pcd_reset(rc522_t *me)
{
    uint32_t err = rc522_write_register(me, REG_COMMAND, PCD_CMD_SOFT_RESET);

    if (err != ERROR_OK) {
        return err;
    }

    /* Soft reset completion is signalled by PowerDown clearing in
     * REG_COMMAND (datasheet 8.8.2) - poll it the same bounded way every
     * other blocking wait in this codebase does (see os_reg_wait_flag_*
     * in Dev/4.MiddleWare/os/os.h), just on a register read instead of a
     * raw volatile pointer since this register lives on the other side
     * of an SPI transfer. */
    uint32_t start_tick = os_tick_get();

    for (;;) {
        uint8_t cmd_reg;
        err = rc522_read_register(me, REG_COMMAND, &cmd_reg);

        if (err != ERROR_OK) {
            return err;
        }

        if ((cmd_reg & 0x10U) == 0U) {
            return ERROR_OK;
        }

        if ((os_tick_get() - start_tick) >= RC522_TIMEOUT_MS) {
            return ERROR_TIMEOUT;
        }
    }
}

uint32_t rc522_init(rc522_t *me, const rc522_init_t *init)
{
    if ((me == NULL) || (init == NULL) || (init->pSpiBus == NULL) ||
        (init->pCsPin == NULL) || (init->pRstPin == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    me->pSpiBus = init->pSpiBus;
    me->pCsPin = init->pCsPin;
    me->pRstPin = init->pRstPin;

    do_set(me->pCsPin);

    /* Hardware reset pulse (datasheet 8.8.1: NRSTPD low >100ns, then
     * wait >=37.74us before the first SPI access after it goes high -
     * both far shorter than this function's own SPI traffic that
     * follows, so no extra delay is needed). */
    do_reset(me->pRstPin);
    do_set(me->pRstPin);

    uint32_t err = rc522_pcd_reset(me);

    if (err != ERROR_OK) {
        return err;
    }

    /* Timer: 40MHz / (TPreScaler+1) ticks/sec, used only for this
     * driver's own TX/RX timeout guard (TAuto=1 so it starts on
     * transmit end) - values from NXP's reference init sequence,
     * ~25us per tick, reload 1000 -> ~25ms timeout. */
    err = rc522_write_register(me, REG_T_MODE, 0x80U);

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_T_PRESCALER, 0xA9U);
    }

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_T_RELOAD_H, 0x03U);
    }

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_T_RELOAD_L, 0xE8U);
    }

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_TX_ASK, 0x40U); /* force 100% ASK */
    }

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_MODE, 0x3DU); /* CRC preset 0x6363 */
    }

    if (err == ERROR_OK) {
        err = rc522_antenna_on(me);
    }

    return err;
}

/*==============================================================================
 * PICC communication
 *============================================================================*/

/*
 * Loads pui8TxBuff into the FIFO, starts a Transceive, waits for RxIRq
 * or a timer timeout, and reads back whatever ended up in the FIFO.
 * ui8TxLastBits: number of valid bits in the last TX byte (0 = all 8,
 * matching REG_BIT_FRAMING's TxLastBits field) - REQA/WUPA are 7-bit
 * frames, everything else here is byte-aligned.
 */
static uint32_t rc522_transceive(rc522_t *me,
                                  const uint8_t *pui8TxBuff, uint32_t ui32TxLength, uint8_t ui8TxLastBits,
                                  uint8_t *pui8RxBuff, uint32_t *pui32RxLength)
{
    uint32_t err = rc522_write_register(me, REG_COMMAND, PCD_CMD_IDLE);

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_COM_IRQ, 0x7FU); /* clear all IRQ flags */
    }

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_FIFO_LEVEL, 0x80U); /* flush FIFO */
    }

    for (uint32_t i = 0U; (err == ERROR_OK) && (i < ui32TxLength); i++) {
        err = rc522_write_register(me, REG_FIFO_DATA, pui8TxBuff[i]);
    }

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_BIT_FRAMING, ui8TxLastBits);
    }

    if (err == ERROR_OK) {
        err = rc522_write_register(me, REG_COMMAND, PCD_CMD_TRANSCEIVE);
    }

    if (err == ERROR_OK) {
        /* BitFramingReg.StartSend must be set after the command, to
         * actually start transmission (datasheet 8.3.1). */
        err = rc522_set_register_bitmask(me, REG_BIT_FRAMING, 0x80U);
    }

    if (err != ERROR_OK) {
        return err;
    }

    uint32_t start_tick = os_tick_get();
    uint8_t irq_reg;

    for (;;) {
        err = rc522_read_register(me, REG_COM_IRQ, &irq_reg);

        if (err != ERROR_OK) {
            return err;
        }

        if ((irq_reg & (COM_IRQ_RX_IRQ_MASK | COM_IRQ_TIMER_IRQ_MASK)) != 0U) {
            break;
        }

        if ((os_tick_get() - start_tick) >= RC522_TIMEOUT_MS) {
            return ERROR_TIMEOUT;
        }
    }

    if ((irq_reg & COM_IRQ_RX_IRQ_MASK) == 0U) {
        return ERROR_TIMEOUT; /* only the timer IRQ fired - no PICC answered */
    }

    uint8_t error_reg;
    err = rc522_read_register(me, REG_ERROR, &error_reg);

    /* BufferOvfl/ParityErr/ProtocolErr - CollErr is handled by the
     * caller (expected during anticollision), not treated as a failure
     * here. */
    if ((err == ERROR_OK) && ((error_reg & 0x13U) != 0U)) {
        return ERROR_FAIL;
    }

    if (err != ERROR_OK) {
        return err;
    }

    uint8_t fifo_level;
    err = rc522_read_register(me, REG_FIFO_LEVEL, &fifo_level);

    if (err != ERROR_OK) {
        return err;
    }

    if (fifo_level > *pui32RxLength) {
        return ERROR_INVALID_PARAM; /* caller's buffer is too small */
    }

    for (uint32_t i = 0U; (err == ERROR_OK) && (i < fifo_level); i++) {
        err = rc522_read_register(me, REG_FIFO_DATA, &pui8RxBuff[i]);
    }

    *pui32RxLength = fifo_level;

    return err;
}

uint32_t rc522_request_a(rc522_t *me)
{
    if (me == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t tx_cmd = PICC_CMD_REQA;
    uint8_t rx_buf[2];
    uint32_t rx_len = sizeof(rx_buf);

    /* REQA is a 7-bit short frame (TxLastBits=7); a 2-byte ATQA answer
     * means a card is present - its contents aren't otherwise used here. */
    return rc522_transceive(me, &tx_cmd, 1U, 0x07U, rx_buf, &rx_len);
}

uint32_t rc522_read_uid(rc522_t *me, uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak)
{
    if ((me == NULL) || (pui8Uid == NULL)) {
        return ERROR_INVALID_PARAM;
    }

    uint32_t err = rc522_request_a(me);

    if (err != ERROR_OK) {
        return err; /* no card in the field */
    }

    /* Anticollision, cascade level 1: send SEL + NVB=0x20 (ask every
     * PICC in the field to answer with its full 4-byte UID + 1 BCC
     * byte, no bits already known) - skips the bit-by-bit collision
     * loop since this driver only supports a single card in the field
     * at a time. */
    uint8_t tx_buf[2] = { PICC_CMD_ANTICOLL_CL1, 0x20U };
    uint8_t rx_buf[5];
    uint32_t rx_len = sizeof(rx_buf);

    err = rc522_transceive(me, tx_buf, 2U, 0x00U, rx_buf, &rx_len);

    if (err != ERROR_OK) {
        return err;
    }

    if (rx_len != 5U) {
        return ERROR_FAIL;
    }

    uint8_t bcc = rx_buf[0] ^ rx_buf[1] ^ rx_buf[2] ^ rx_buf[3];

    if (bcc != rx_buf[4]) {
        return ERROR_FAIL;
    }

    pui8Uid[0] = rx_buf[0];
    pui8Uid[1] = rx_buf[1];
    pui8Uid[2] = rx_buf[2];
    pui8Uid[3] = rx_buf[3];

    /* SELECT, cascade level 1: send the UID back with NVB=0x70 (full
     * SELECT, no CRC appended by us - the PCD's CRC coprocessor isn't
     * invoked here, and the PICC's SAK answer includes its own CRC that
     * this driver doesn't verify) to complete selection and get the SAK
     * byte identifying the PICC type. */
    uint8_t select_buf[7] = { PICC_CMD_ANTICOLL_CL1, 0x70U,
                               rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3], bcc };
    uint8_t sak_buf[3];
    uint32_t sak_len = sizeof(sak_buf);

    err = rc522_transceive(me, select_buf, 7U, 0x00U, sak_buf, &sak_len);

    if ((err == ERROR_OK) && (pui8Sak != NULL) && (sak_len >= 1U)) {
        *pui8Sak = sak_buf[0];
    }

    return err;
}

uint32_t rc522_halt(rc522_t *me)
{
    if (me == NULL) {
        return ERROR_INVALID_PARAM;
    }

    uint8_t tx_buf[2] = { PICC_CMD_HALTA, 0x00U };
    uint8_t rx_buf[1];
    uint32_t rx_len = sizeof(rx_buf);

    /* A halted PICC doesn't answer at all, so this normally returns
     * ERROR_TIMEOUT even on success - that's expected, not a failure. */
    (void)rc522_transceive(me, tx_buf, 2U, 0x00U, rx_buf, &rx_len);

    return ERROR_OK;
}
