/*
 * Copyright (C) 2014 Loci Controls Inc.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     board_cc2538dk
 * @{
 *
 * @file        board.c
 * @brief       Board specific implementations for the CC2538DK board
 *
 * @author      Ian Martin <ian@locicontrols.com>
 */

#include <stdio.h>

#include "board.h"
#include "cpu.h"

#include "lpm.h"
#include "cc2538-gpio.h"

/**
 * @brief Initialize the SmartRF06's on-board LEDs
 */
void led_init(void)
{
    gpio_init_out(LED_RED_GPIO,    GPIO_NOPULL);
    gpio_init_out(LED_GREEN_GPIO,  GPIO_NOPULL);
    gpio_init_out(LED_YELLOW_GPIO, GPIO_NOPULL);
    gpio_init_out(LED_ORANGE_GPIO, GPIO_NOPULL);
}

/**
 * @brief Initialize the SmartRF06 board
 */
void board_init(void)
{
    /* initialize core clocks via a CMSIS function */
    SystemInit();

    /* initialize the CPU */
    cpu_init();

    /* initialize the boards LEDs */
    led_init();
}

/** @} */
