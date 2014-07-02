/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    board_arduino-due Arduino Due
 * @ingroup     boards
 * @brief       Support for the Arduino Due board.
 * @{
 *
 * @file        board.h
 * @brief       Board specific definitions for the Arduino Due board.
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef __BOARD_H
#define __BOARD_H

#include "cpu.h"

/**
 * Define the nominal CPU core clock in this board
 */
#define F_CPU               (84000000UL)

/**
 * Define the type for the radio packet length for the transceiver
 */
typedef uint8_t radio_packet_length_t;

/**
 * Assign the hardware timer
 */
#define HW_TIMER            TIMER_0

/**
 * @name LED pin definitions
 * @{
 */
#define LED_PORT            PIOB
#define LED_PIN             PIO_PB27
/** @} */

/**
 * @name Macros for controlling the on-board LEDs.
 * @{
 */
#define LED_ON              LED_PORT->PIO_ODSR |= LED_PIN
#define LED_OFF             LED_PORT->PIO_ODSR &= ~LED_PIN
#define LED_TOGGLE          LED_PORT->PIO_ODSR ^= LED_PIN;

/* for compatability to other boards */
#define LED_GREEN_ON        LED_ON
#define LED_GREEN_OFF       LED_OFF
#define LED_GREEN_TOGGLE    LED_TOGGLE
#define LED_RED_ON          /* not available */
#define LED_RED_OFF         /* not available */
#define LED_RED_TOGGLE      /* not available */
/** @} */


/**
 * @brief Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);


#endif /** __BOARD_H */
/** @} */
