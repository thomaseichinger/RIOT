/*
 * Copyright (C) 2014 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "at86rf231_spi.h"
#include "at86rf231_arch.h"
#include "at86rf231.h"
#include "periph/spi.h"
#include "periph_conf.h"

void at86rf231_reg_write(uint8_t addr, uint8_t value)
{
    /* Start the SPI transfer */
    at86rf231_spi_select();

    spi_transfer_reg(SPI_0, AT86RF231_ACCESS_REG | AT86RF231_ACCESS_WRITE | addr, value, NULL);

    /* End the SPI transfer */
    at86rf231_spi_unselect();
}

uint8_t at86rf231_reg_read(uint8_t addr)
{
    uint8_t value;

    /* Start the SPI transfer */
    at86rf231_spi_select();

    spi_transfer_reg(SPI_0, AT86RF231_ACCESS_REG | AT86RF231_ACCESS_READ | addr, 0, (char*)(&value));

    /* End the SPI transfer */
    at86rf231_spi_unselect();

    return value;
}

void at86rf231_read_fifo(uint8_t *data, radio_packet_length_t length)
{
    /* Start the SPI transfer */
    at86rf231_spi_select();

    /* Send Frame Buffer read access */
    spi_transfer_byte(SPI_0, AT86RF231_ACCESS_FRAMEBUFFER | AT86RF231_ACCESS_READ, NULL);

    /* access frame buffer */
    spi_transfer_bytes(SPI_0, NULL, (char*)data, (unsigned int) length);

    /* End the SPI transfer */
    at86rf231_spi_unselect();
}

void at86rf231_write_fifo(const uint8_t *data, radio_packet_length_t length)
{
    /* Start the SPI transfer */
    at86rf231_spi_select();

    /* Send Frame Buffer Write access */
    spi_transfer_byte(SPI_0, AT86RF231_ACCESS_FRAMEBUFFER | AT86RF231_ACCESS_WRITE, NULL);

    /* access frame buffer */
    spi_transfer_bytes(SPI_0, (char*)data, NULL, (unsigned int)length);

    /* End the SPI transfer */
    at86rf231_spi_unselect();
}
