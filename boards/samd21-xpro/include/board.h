/*
 * Copyright (C) 2017 Thomas Eichinger
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_samd21-xpro Atmel SAM D21 Xplained Pro
 * @ingroup     boards
 * @brief       Support for the Atmel SAM D21 Xplained Pro board.
 * @{
 *
 * @file
 * @brief       Board specific definitions for the Atmel SAM D21 Xplained Pro
 *              board
 *
 * @author      Thomas Eichinger <thomas@riot-os.org>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "periph_conf.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   xtimer configuration
 * @{
 */
#define XTIMER_DEV          TIMER_1
#define XTIMER_CHAN         (0)
/** @} */

/**
 * @brief   LED pin definitions and handlers
 * @{
 */
#define LED0_PIN            GPIO_PIN(1, 30)

#define LED_PORT            PORT->Group[1]
#define LED0_MASK           (1 << 30)

#define LED0_ON             (LED_PORT.OUTCLR.reg = LED0_MASK)
#define LED0_OFF            (LED_PORT.OUTSET.reg = LED0_MASK)
#define LED0_TOGGLE         (LED_PORT.OUTTGL.reg = LED0_MASK)
/** @} */

/**
 * @name SW0 (Button) pin definitions
 * @{
 */
#define BUTTON_PORT         PORT->Group[0]
#define BUTTON_PIN          (15)
#define BUTTON_GPIO         GPIO_PIN(0, BUTTON_PIN)
/** @} */

/**
 * @brief Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
