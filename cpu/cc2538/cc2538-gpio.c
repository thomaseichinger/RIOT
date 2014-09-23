/*
 * Copyright (C) 2014 Loci Controls Inc.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file            cc2538-gpio.c
 * @brief           Implementation of the cc2538 GPIO controller
 *
 * @author          Ian Martin <ian@locicontrols.com>
 *
 * @addtogroup cc2538-gpio
 * @{
 */
#include "cc2538-gpio.h"

#include "cpu.h"

#include <string.h>

/** @brief Interrupt service routine for Port A */
__attribute__((naked))
void gpio_port_a_isr(void)
{
    GPIO_A->IC             = 0x000000ff;
    GPIO_A->IRQ_DETECT_ACK = 0x000000ff;
}

/** @brief Interrupt service routine for Port B */
__attribute__((naked))
void gpio_port_b_isr(void)
{
    GPIO_B->IC             = 0x000000ff;
    GPIO_B->IRQ_DETECT_ACK = 0x0000ff00;
}

/** @brief Interrupt service routine for Port C */
__attribute__((naked))
void gpio_port_c_isr(void)
{
    GPIO_C->IC             = 0x000000ff;
    GPIO_C->IRQ_DETECT_ACK = 0x00ff0000;
}

/** @brief Interrupt service routine for Port D */
__attribute__((naked))
void gpio_port_d_isr(void)
{
    GPIO_D->IC             = 0x000000ff;
    GPIO_D->IRQ_DETECT_ACK = 0xff000000;
}

/** @} */
/** @} */
