/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    driver_pn532 NXP pn532 NFC module
 * @ingroup     drivers
 * @brief       Device driver for the NXP pn532 NFC module
 * @{
 *
 * @file
 * @brief       Device driver for the NXP pn532 NFC module
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef __PN532_H
#define __PN532_H

#include "periph/i2c.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The modules default I2C address
 */
#define PN532_DEFAULT_ADDRESS   (0x48)

/**
 * @brief Device descriptor for PN532 module
 */
typedef struct {
    i2c_t   i2c;        /**< I2C device                  */
    uint8_t address;    /**< module's i2c address        */
    gpio_t  pin;        /**< module's response ready pin */
} pn532_t;

/**
 * @brief Commands used for transactions with the PN532 module
 */
typedef enum {
    pn532_cmd_diagnose = 0x00,
} pn532_cmd_t;

/**
 * @brief Initialise a new PN532 device
 *
 * @param[in] dev   device descriptor of a PN532 device
 * @param[in] i2c   I2C device the module is connected to
 * @param[in] pin   GPIO pin connected to the module
 * @param[in] addr  module's I2C address
 *
 * @return  0 on success
 * @return -1 on error
 */
int pn532_init(pn532_t *dev, i2c_t i2c, gpio_t pin, uint8_t address);

/**
 * @brief Transfer command to the pn532 and get the response
 *
 * @param[in]  dev  device descriptor of a PN532 device
 * @param[in]  cmd  command sent to the module
 * @param[in]  out  buffer holding the commands parameter
 * @param[out] in   buffer the response is written to
 *
 * @return  0 on success
 * @return -1 on error
 * @return -2 if `in_len` is smaller than the received data
 */
int pn532_transaction(pn532_t *dev, pn532_cmd_t cmd,
                      uint8_t *out, uint8_t out_len,
                      uint8_t *in,  uint8_t in_len);

#endif /* __cplusplus */

#endif /* __PN532_H */

/** @} */