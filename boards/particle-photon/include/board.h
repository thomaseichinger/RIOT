/*
 * Copyright (C) 2016  Thomas Eichinger
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_particle-photon Particle Photon
 * @ingroup     boards
 * @brief       Board specific files for the Particle Photon board
 * @{
 *
 * @file
 * @brief       Board specific definitions for the Particle Photon board
 *
 * @author      Thomas Eichinger <thomas@riot-os.org>
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "board_common.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief   LED pin definitions and handlers
 * @{
 */
#undef LED0_PIN
#undef LED0_MASK
#undef LED0_ON
#undef LED0_OFF
#undef LED0_TOGGLE

#define LED0_PIN            GPIO_PIN(PORT_B, 0)
#define LED0_MASK           (1 << 0)
#define LED0_ON             (GPIOB->BSRR = LED0_MASK)
#define LED0_OFF            (GPIOB->BSRR = (LED0_MASK << 16))
#define LED0_TOGGLE         (GPIOB->ODR  ^= LED0_MASK)
/** @} */

/**
 * @brief Use the 1st UART for STDIO on this board
 */
#define UART_STDIO_DEV      UART_DEV(0)

/**
 * @brief   User button
 */
#define BTN_B1_PIN          GPIO_PIN(PORT_C, 13)

/**
 * @brief   Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */
/** @} */
