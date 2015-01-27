/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_pn532
 * @{
 *
 * @file
 * @brief       Device driver for the NXP pn532 NFC module
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * @}
 */

#include "thread.h"

#include "pn532.h"
#include "pn532_internal.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

int pn532_init(pn532_t *dev, i2c_t i2c, gpio_t pin, uint8_t address)
{
    dev->i2c = i2c;
    dev->pin = pin;
    dev->address = address;

    i2c_acquire(i2c);
    i2c_init_master(i2c, I2C_SPEED_NORMAL);
    i2c_release(i2c);
    DEBUG("%s: i2c device initialised.\n", __PRETTY_FUNCTION__);

    gpio_init_in(dev->pin, GPIO_NOPULL);

    DEBUG("%s: start communication line test.\n", __PRETTY_FUNCTION__);
    uint8_t out_buf = { 0x01, 0x08, 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07 };
    uint8_t in_buf[10];

    pn532_transcaction(dev, pn532_cmd_diagnose, out_buf, in_buf);
    int res;
    for (int i = 0; i < 10; i++) {
        DEBUG("out: %02x in: %02x\n", out_buf[i], in_buf[i]);
        if (out_buf[i] != in_buf[i]) {
            DEBUG("%s: [ERROR]\n");
            return -1;
        }
    }
    DEBUG("%s: Finished communication line test.\n", __PRETTY_FUNCTION__);

    return 0;
}

int pn532_transcaction(pn532_t *dev, pn532_cmd_t cmd,
                       uint8_t *out, uint8_t out_len,
                       uint8_t *in,  uint8_t in_len)
{
    uint8_t buff[255];
    buff[0] = PN532_FRAME_PRE_POSTAMBLE;
    buff[1] = PN532_FRAME_SOPC_0;
    buff[2] = PN532_FRAME_SOPC_1;
    buff[3] = out_len + 1;
    buff[4] = (~buff[3]) + 1;
    buff[5] = PN532_FRAME_TFI_FROM_HOST;

    uint8_t acc = 0;
    for (int i = 0; i < out_len; i++) {
        buff[PN532_NORM_DATA_OFFSET + i] = out_buf[i];
        acc += out_buf[i];
    }

    buff[PN532_NORM_DATA_OFFSET + out_len] = (~acc) + 1;
    buff[PN532_NORM_DATA_OFFSET + out_len + 1] = PN532_FRAME_PRE_POSTAMBLE;

    i2c_acquire(dev->i2c);
    i2c_write_bytes(dev->i2c, dev->address, (char*)buff, PN532_NORM_DATA_OFFSET + out_len + 2);
    i2c_release(dev->i2c);

    while(!gpio_read(dev->pin)) {
        thread_yield();
    }

    i2c_acquire(dev->i2c);
    i2c_read_bytes(dev->i2c, dev->address, (char*)buff, 5);
    i2c_read_bytes(dev->i2c, dev->address, (char*)(&buff[5]), buff[4]+2);
    i2c_release(dev->i2c);

    if (in_len < buff[4]) {
        return -2;
    }

    acc = 0;
    for (int i = 0; i < in_len; i++) {
        in_buf[i] = buff[PN532_NORM_DATA_OFFSET + i];
        acc += in_buf[i];
    }

    if ((uint8_t)(acc + 1 + buff[buff[PN532_FRAME_POS_LEN]])) {
        return -1;
    }

    return 0;

}