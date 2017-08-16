/*
 * Copyright (C)  2016 Freie Universität Berlin
 *                2016 Inria
 *                2017 Thomas Eichinger
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_rs-mini-ultra-pro-v2
 * @{
 *
 * @file
 * @brief       Board specific implementations for the Rocket Scream Mini Ultra Pro v2 board
 *
 * @author      Hauke Pertersen  <hauke.pertersen@fu-berlin.de>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author      Thomas Eichinger <thomas@riot-os.org>
 *
 * @}
 */

#include "cpu.h"
#include "board.h"
#include "periph/gpio.h"

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();
    /* initialize the on-board orange LED on pin PD13 */
    gpio_init(LED0_PIN, GPIO_OUT);
}
