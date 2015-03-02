/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    boards_xmc4500-rk XMC4500 Relax Kit
 * @ingroup     boards
 * @brief       Board specific files for the Infineon XMC4500 Relax Kit
 * @{
 *
 * @file
 * @brief       Board specific definitions for the Infineon XMC4500 Relax Kit
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef __BOARD_H
#define __BOARD_H

#include "cpu.h"
#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define the nominal CPU core clock in this board
 */
#define F_CPU               CLOCK_CORECLOCK

/**
 * @name Assign the hardware timer
 */
#define HW_TIMER            TIMER_0

/**
 * @name Define UART device and baudrate for stdio
 * @{
 */
#define STDIO               UART_0
#define STDIO_BAUDRATE      (115200U)
#define STDIO_RX_BUFSIZE    (64U)
/** @} */

/**
 * @name LED pin definitions
 * @{
 */
#define LED_PORT            PORT1
#define RED_PIN             (1 << 0)
#define GREEN_PIN           (1 << 1)
/** @} */


/* for compatability to other boards */
#define LED_GREEN_ON        (LED_PORT->OMR = GREEN_PIN)
#define LED_GREEN_OFF       (LED_PORT->OMR = (GREEN_PIN << 16))
#define LED_GREEN_TOGGLE    (LED_PORT->OMR = (GREEN_PIN | (GREEN_PIN << 16))
#define LED_RED_ON          (LED_PORT->OMR = RED_PIN)
#define LED_RED_OFF         (LED_PORT->OMR = (RED_PIN << 16))
#define LED_RED_TOGGLE      (LED_PORT->OMR = (RED_PIN | (RED_PIN << 16))
/** @} */

/**
 * @brief Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /** __BOARD_H */
/** @} */
