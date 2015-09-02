/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_msp430fxyz
 * @{
 *
 * @file
 * @brief       Low-level SPI driver implementation
 *
 * This SPI driver implementation does only support one single SPI device for
 * now. This is sufficient, as most MSP430 CPU's only support two serial
 * devices - one used as UART and one as SPI.
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "mutex.h"
#include "periph_cpu.h"
#include "periph_conf.h"
#include "periph/spi.h"

/**
 * @brief   Mutex for locking the SPI device
 */
static mutex_t spi_lock = MUTEX_INIT;


int spi_init_master(spi_t dev, spi_conf_t conf, spi_speed_t speed)
{
    if (dev != 0) {
        return -2;
    }

    /* reset SPI device */
    SPI_DEV->CTL = USART_CTL_SWRST;
    /* configure pins */
    spi_conf_pins(dev);
    /* configure USART to SPI mode with SMCLK driving it */
    SPI_DEV->CTL |= (USART_CTL_CHAR | USART_CTL_SYNC | USART_CTL_MM);
    SPI_DEV->RCTL = 0;
    SPI_DEV->TCTL = (USART_TCTL_SSEL_SMCLK | USART_TCTL_STC);
    /* set polarity and phase */
    switch (conf) {
        case SPI_CONF_SECOND_RISING:
            SPI_DEV->TCTL |= USART_TCTL_CKPH;
            break;
        case SPI_CONF_FIRST_FALLING:
            SPI_DEV->TCTL |= SPI_CONF_FIRST_FALLING;
            break;
        case SPI_CONF_SECOND_FALLING:
            SPI_DEV->TCTL |= (USART_TCTL_CKPH | SPI_CONF_FIRST_FALLING);
            break;
        default:
            /* do nothing */
            break;
    }
    /* configure clock - we use no modulation for now */
    uint32_t br = CLOCK_CMCLK;
    switch (speed) {
        case SPI_SPEED_100KHZ:
            br /= 100000;
        case SPI_SPEED_400KHZ:
            br /= 400000;
        case SPI_SPEED_1MHZ:
            br /= 1000000;
        case SPI_SPEED_5MHZ:
            br /= 5000000;
            if (br < 2) {       /* make sure the is not smaller then 2 */
                br = 2;
            }
            break;
        default:
            /* other clock speeds are not supported */
            return -1;
    }
    SPI_DEV->BR0 = (uint8_t)br;
    SPI_DEV->BR1 = (uint8_t)(br >> 8);
    SPI_DEV->MCTL = 0;
    /* enable SPI mode */
    SPI_ME |= SPI_ME_BIT;
    /* release from software reset */
    SPI_DEV->CTL &= ~(USART_CTL_SWRST);
    return 0;
}

int spi_init_slave(spi_t dev, spi_conf_t conf, char (*cb)(char data))
{
    /* not supported so far */
    (void)dev;
    (void)conf;
    (void)cb;
    return -1;
}

void spi_transmission_begin(spi_t dev, char reset_val)
{
    /* not supported so far */
    (void)dev;
    (void)reset_val;
}

int spi_conf_pins(spi_t dev)
{
    (void)dev;
    gpio_periph_mode(SPI_PIN_MISO, true);
    gpio_periph_mode(SPI_PIN_MOSI, true);
    gpio_periph_mode(SPI_PIN_CLK, true);
    return 0;
}

int spi_acquire(spi_t dev)
{
    (void)dev;
    mutex_lock(&spi_lock);
    return 0;
}

int spi_release(spi_t dev)
{
    (void)dev;
    mutex_unlock(&spi_lock);
    return 0;
}

int spi_transfer_byte(spi_t dev, char out, char *in)
{
    (void)dev;
    char tmp;
    while (!(SPI_IE & SPI_IE_TX_BIT));
    SPI_DEV->TXBUF = (uint8_t)out;
    while (!(SPI_IE & SPI_IE_RX_BIT));
    tmp = (char)SPI_DEV->RXBUF;
    if (in) {
        *in = tmp;
    }
    return 1;
}

void spi_poweron(spi_t dev)
{
    /* not supported so far */
    (void)dev;
}

void spi_poweroff(spi_t dev)
{
    /* not supported so far */
    (void)dev;
}
