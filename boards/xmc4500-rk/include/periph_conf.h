/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     boards_xmc4500-rk
 * @{
 *
 * @file
 * @name       Peripheral MCU configuration for the Infineon XMC4500 Relax Kit
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef __PERIPH_CONF_H
#define __PERIPH_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Clock system configuration
 * @{
 */
#define CLOCK_HSE           (8000000U)          /* external oscillator */
#define CLOCK_CORECLOCK     (168000000U)        /* desired core clock frequency */

/* the actual PLL values are automatically generated */
#define CLOCK_PLL_M         (CLOCK_HSE / 1000000)
#define CLOCK_PLL_N         ((CLOCK_CORECLOCK / 1000000) * 2)
#define CLOCK_PLL_P         (2U)
#define CLOCK_PLL_Q         (CLOCK_PLL_N / 48)
#define CLOCK_AHB_DIV       RCC_CFGR_HPRE_DIV1
#define CLOCK_APB2_DIV      RCC_CFGR_PPRE2_DIV2
#define CLOCK_APB1_DIV      RCC_CFGR_PPRE1_DIV4
#define CLOCK_FLASH_LATENCY FLASH_ACR_LATENCY_5WS
/** @} */

#define UART_NUMOF          (1)

#define UART_0_EN           (1)

#define UART_IRQ_PRIO       1

#define UART_0_DEV          (USIC0_CH0)
#define UART_0_TX_PIN_REG   (PORT0->IOCR0)
#define UART_0_RX_PIN_REG   (PORT0->IOCR0)
#define UART_0_TX_PIN       (1)
#define UART_0_RX_PIN       (0)
#define UART_0_AF           (0x02)
#define UART_0_RX_SIG_REG   (USIC0_CH0->DX0CR)
#define UART_0_RX_REG       (0x03)
#define UART_0_IRQ_CHAN     USIC0_0_IRQn
#define UART_0_IRQ_SR       0

#ifdef __cplusplus
}
#endif

#endif /* __PERIPH_CONF_H */
/** @} */