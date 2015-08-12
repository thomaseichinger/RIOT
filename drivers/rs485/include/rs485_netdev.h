/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_rs485
 * @{
 *
 * @file
 * @brief       Netdev interface to RS485 drivers
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef RS485_NETDEV_H_
#define RS485_NETDEV_H_

#include "net/ng_netdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Reference to the netdev device driver struct
 */
extern const ng_netdev_driver_t rs485_driver;

#ifdef __cplusplus
}
#endif

#endif /* RS485_NETDEV_H_ */
/** @} */