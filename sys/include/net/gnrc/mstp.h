/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_mstp  Master Slave Token Passing MAC protocol for RS485
 * @ingroup     net_gnrc
 * @brief       Master Slave Token Passing MAC protocol for RS485
 * @{
 *
 * @file
 * @brief       Interface definition for the MS/TP MAC layer
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef GNRC_MSTP_H_
#define GNRC_MSTP_H_

#include "periph/uart.h"
#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Set the default message queue size for MSTP layers
 */
#ifndef GNRC_MSTP_MSG_QUEUE_SIZE
#define GNRC_MSTP_MSG_QUEUE_SIZE       (8U)
#endif

typedef struct
{
    uint8_t hdr_index;
    uint8_t valid;
    uint8_t type;
    uint8_t dst_addr;
    uint8_t src_addr;
    uint16_t length;
    uint8_t encoded;
    uint8_t header_crc;
    uint8_t data[1500];
    uint16_t data_crc;
} mstp_frame_t;

typedef struct
{
    uart_t uart;
    kernel_pid_t mac_pid;
    uint8_t state;
    uint8_t addr;
    mstp_frame_t frame;
    uint16_t options;
} gnrc_mstp_t;

/**
 * @brief struct holding all params needed for device initialization
 */
typedef struct gnrc_mstp_params {
    uart_t uart;            /**< UART device is to use */
    uint32_t baudrate;      /**< Baudrate to use */
} gnrc_mstp_params_t;

int gnrc_mstp_init(gnrc_mstp_t * ctx, const gnrc_mstp_params_t *p);

/**
 * @brief   Initialize an instance of the MSTP layer
 *
 * The initialization starts a new thread that connects to the given MSTP
 * context and starts a link layer event loop.
 *
 * @param[in] stack         stack for the control thread
 * @param[in] stacksize     size of *stack*
 * @param[in] priority      priority for the thread housing the MSTP instance
 * @param[in] name          name of the thread housing the MSTP instance
 * @param[in] ctx           MSTP context, needs to be already initialized
 * @param[in] p             Parameters to initialize MSTP with
 *
 * @return                  PID of NOMAC thread on success
 * @return                  -EINVAL if creation of thread fails
 * @return                  -ENODEV if *dev* is invalid
 */
kernel_pid_t gnrc_mstp_start(char *stack, int stacksize, char priority,
                             const char *name, gnrc_mstp_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* GNRC_MSTP_H_ */
/** @} */
