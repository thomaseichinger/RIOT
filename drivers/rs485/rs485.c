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
 * @brief       Implementation of public functions for RS485 drivers
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * @}
 */

#include "periph/uart.h"

static void _irq_handler(void *arg)
{
    msg_t msg;
    rs485_t *dev = (rs485_t *) arg;

    /* tell driver thread about the interrupt */
    msg.type = NG_NETDEV_MSG_TYPE_EVENT;
    msg_send(&msg, dev->mac_pid);
}

int rs485_init(rs485_t *dev, uart_t uart, uint32_t baudrate, 
               rs485_packet_buffer_t rx, rs485_packet_buffer_t tx)
{
    dev->driver = &rs485_driver;

    /* initialize device descriptor */
    dev->uart = uart;
    dev->baudrate = baudrate;
    dev->state = RS485_STATE_IDLE;
    dev->rx_buffer = rx;
    dev->tx_buffer = tx;

    /* set default protocol */
#ifdef MODULE_NG_SIXLOWPAN
    dev->proto = NG_NETTYPE_SIXLOWPAN;
#else
    dev->proto = NG_NETTYPE_UNDEF;
#endif

    return uart_init(uart, baudrate, _irq_handler, _irq_handler, (void *) dev); 
}

void rs485_reset(rs485_t *dev)
{
    uart_poweroff(dev->uart);
    uart_poweron(dev->uart);
    rs485_init(dev, dev->uart, dev->baudrate, dev->rx_buffer, dev->tx_buffer);
}

