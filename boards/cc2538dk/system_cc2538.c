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
 * @file            system_cc2538.c
 * @brief           CC2538 CMSIS function implementation
 *
 * @author          Ian Martin <ian@locicontrols.com>
 */

#include "cc2538.h"
#include "cc2538-gpio.h"
#include "board.h"
#include "ioc.h"
#include "sys-ctrl.h"
#include "system_cc2538.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t SystemCoreClock = RCOSC16M_FREQ;

/**
 * @brief Set up the microcontroller system.
 * Initialize the System and update the SystemFrequency variable.
 */
void SystemInit(void)
{
#if SYS_CTRL_OSC32K_USE_XTAL
    /* Set the XOSC32K_Q pads to analog for the external crystal: */
    gpio_software_control(GPIO_PD6);
    gpio_dir_input(GPIO_PD6);
    IOC_PXX_OVER[GPIO_PD6] = IOC_OVERRIDE_ANA;

    gpio_software_control(GPIO_PD7);
    gpio_dir_input(GPIO_PD7);
    IOC_PXX_OVER[GPIO_PD7] = IOC_OVERRIDE_ANA;
#endif

    SYS_CTRL->CLOCK_CTRL             = 0;
    SYS_CTRL->CLOCK_CTRLbits.OSC_PD  = 1; /**< Power down the oscillator not selected by OSC bit (hardware-controlled when selected). */
    SYS_CTRL->CLOCK_CTRLbits.IO_DIV  = 1; /**< 16 MHz */
    SYS_CTRL->CLOCK_CTRLbits.SYS_DIV = 1; /**< 16 MHz */

#if SYS_CTRL_OSC32K_USE_XTAL
    SYS_CTRL->CLOCK_CTRLbits.OSC32K  = 0; /**< Use external 32-kHz crystal oscillator on pads PD6 and PD7. */
#endif

    /* Wait for the new clock settings to take effect: */
    while (SYS_CTRL->CLOCK_STAbits.OSC32K || SYS_CTRL->CLOCK_STAbits.OSC);

#if SYS_CTRL_OSC32K_USE_XTAL
    /* Wait for the 32-kHz crystal oscillator to stabilize: */
    while ( SYS_CTRL->CLOCK_STAbits.SYNC_32K);
    while (!SYS_CTRL->CLOCK_STAbits.SYNC_32K);
#endif

    SystemCoreClockUpdate();
}

/**
 * @brief Update SystemCoreClock with the current clock frequency
 */
void SystemCoreClockUpdate(void)
{
    /* Determine clock frequency according to clock register values */
    SystemCoreClock = sys_clock_freq();
}

#ifdef __cplusplus
}
#endif

/* @} */
