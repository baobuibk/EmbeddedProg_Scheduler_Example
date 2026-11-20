/*
 * bsp_rc522.c
 */

#include "bsp_rc522.h"
#include "rc522.h"
#include "spi_io.h"
#include "dio.h"

#define BSP_RC522_GPIO_PORT_A   (1u)   /* matches dio.h's port numbering (1=GPIOA) */
#define BSP_RC522_GPIO_PORT_B   (2u)   /* matches dio.h's port numbering (2=GPIOB) */

static spi_io_t s_spi2 = { .ui32SpiPort = 2U };
static dio_t s_csPin  = { .port = BSP_RC522_GPIO_PORT_A, .pin = 15U };
static dio_t s_rstPin = { .port = BSP_RC522_GPIO_PORT_B, .pin = 0U };
static rc522_t s_rc522;

void bsp_rc522_init(void)
{
    spi_io_init(&s_spi2);

    dio_mode(&s_csPin, OUTPUT);
    do_set(&s_csPin); /* CS idle high */

    dio_mode(&s_rstPin, OUTPUT);

    rc522_init_t init = { .pSpiBus = &s_spi2, .pCsPin = &s_csPin, .pRstPin = &s_rstPin };

    rc522_init(&s_rc522, &init);
}

uint32_t bsp_rc522_poll(void)
{
    return rc522_request_a(&s_rc522);
}

uint32_t bsp_rc522_read_uid(uint8_t pui8Uid[RC522_UID_LEN], uint8_t *pui8Sak)
{
    return rc522_read_uid(&s_rc522, pui8Uid, pui8Sak);
}

uint32_t bsp_rc522_halt(void)
{
    return rc522_halt(&s_rc522);
}
