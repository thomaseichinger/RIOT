/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_rs485
 * @{
 *
 * @file
 * @brief       Internal interfaces for RS485 drivers
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef RS485_INTERNAL_H_
#define RS485_INTERNAL_H_

#include <stdint.h>

#include "rs485.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get length of data in device's rx buffer.
 * 
 * @param[in] dev   device to read from
 * 
 * @return          the length of received data.
 */
 size_t rs485_rx_len(rs485_t *dev);

/**
 * @brief Read data from the device's rx buffer
 * 
 * @param[in]  dev      device to read from
 * @param[out] data     buffer to read data into
 * @param[in]  len      number of bytes to read
 * @param[in]  offset   offset in buffer to start reading
 */
void rs485_rx_read(rs485_t *dev, uint8_t *data, size_t len,
                   size_t offset)

#ifdef __cplusplus
}
#endif

#endif /* RS485_INTERNAL_H_ */
/** @} */
