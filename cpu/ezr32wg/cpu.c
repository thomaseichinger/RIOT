/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_ezr32wg
 * @{
 *
 * @file
 * @brief       Implementation of the CPU initialization
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @}
 */

#include "cpu.h"
#include "periph_conf.h"

/**
 * @brief   Configure clock sources and the CPU frequency
 */
static void clk_init(void)
{
    /* TODO */

    /* enable high frequency peripheral clock with prescale factor 0 */
    CMU->HFPERCLKDIV = CMU_HFPERCLKDIV_HFPERCLKEN;
}

void cpu_init(void)
{
    /* initialize the Cortex-M core */
    cortexm_init();
    /* Initialise clock sources and generic clocks */
    clk_init();
}
