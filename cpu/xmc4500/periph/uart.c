/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_xmc4500
 * @{
 *
 * @file
 * @brief       Low-level UART driver implementation
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "thread.h"
#include "sched.h"
#include "periph_conf.h"
#include "periph/uart.h"

/* guard file in case no UART device was specified */
#if UART_NUMOF
/**
 * @brief Each UART device has to store two callbacks.
 */
typedef struct {
    uart_rx_cb_t rx_cb;
    uart_tx_cb_t tx_cb;
    void *arg;
} uart_conf_t;

/**
 * @brief Unified interrupt handler for all UART devices
 *
 * @param uartnum       the number of the UART that triggered the ISR
 * @param uart          the UART device that triggered the ISR
 */
static inline void irq_handler(uart_t uartnum, USIC_CH_TypeDef *uart);

/**
 * @brief Allocate memory to store the callback functions.
 */
static uart_conf_t uart_config[UART_NUMOF];

int uart_init(uart_t uart, uint32_t baudrate, uart_rx_cb_t rx_cb, uart_tx_cb_t tx_cb, void *arg)
{
    /* do basic initialization */
    int res = uart_init_blocking(uart, baudrate);
    if (res < 0) {
        return res;
    }

    /* remember callback addresses */
    uart_config[uart].rx_cb = rx_cb;
    uart_config[uart].tx_cb = tx_cb;
    uart_config[uart].arg = arg;

    /* enable receive interrupt */
    switch (uart) {
#if UART_0_EN
        case UART_0:
            NVIC_SetPriority(UART_0_IRQ_CHAN, UART_IRQ_PRIO);
            NVIC_EnableIRQ(UART_0_IRQ_CHAN);
            UART_0_DEV->PCR |= 0x00000080;
            UART_0_DEV->INPR |= UART_0_IRQ_SR;
            break;
#endif
    }

    return 0;
}

int uart_init_blocking(uart_t uart, uint32_t baudrate)
{
    USIC_CH_TypeDef *dev = 0;
    uint32_t tx_pin = 0;
    uint32_t rx_pin = 0;
    uint8_t af = 0;
    volatile uint32_t *tx_pin_reg = 0;
    volatile uint32_t *rx_pin_reg = 0;
    volatile uint32_t *rx_sig_reg = 0;
    uint8_t rx_reg = 0; 

    switch (uart) {
#if UART_0_EN
        case UART_0:
            dev = UART_0_DEV;
            tx_pin = UART_0_TX_PIN;
            rx_pin = UART_0_RX_PIN;
            af = UART_0_AF;
            tx_pin_reg = &UART_0_TX_PIN_REG;
            rx_pin_reg = &UART_0_RX_PIN_REG;
            rx_sig_reg = &UART_0_RX_SIG_REG;
            rx_reg = UART_0_RX_REG;
            break;
#endif
    }

    /* output, opendrain and alterante function */
    *tx_pin_reg |= (0x18 | af) << tx_pin;
    /* input, no pull */
    *rx_pin_reg |= (0x00) << rx_pin;

    /* disable channel during configuration */
    dev->CCR &= ~USIC_CH_CCR_MODE_Msk;
    /* set TRM to 1, word length to 8 */
    dev->SCTR |= 0x08000100;
    /* enable INSW, set input signal */
    *rx_sig_reg |= (0x00000010 | rx_reg);

    /* enable channel */
    dev->CCR |= 0x2;

    return 0;
}

void uart_tx_begin(uart_t uart)
{
    switch (uart) {
#if UART_0_EN
        case UART_0:
            UART_0_DEV->PCR |= 0x00000080;
            break;
#endif
    }
}

int uart_write(uart_t uart, char data)
{
    USIC_CH_TypeDef *dev = 0;

    switch (uart) {
#if UART_0_EN
        case UART_0:
            dev = UART_0_DEV;
            break;
#endif
    }

    if (dev->PSR & 0x00000001) {
        dev->TBUF[0] = (uint8_t)data;
    }

    return 0;
}

int uart_read_blocking(uart_t uart, char *data)
{
    USIC_CH_TypeDef *dev = 0;

    switch (uart) {
#if UART_0_EN
        case UART_0:
            dev = UART_0_DEV;
            break;
#endif
    }

    while (!(dev->PSR & 0x00000002));
    *data = (char)dev->RBUF;

    return 1;
}

int uart_write_blocking(uart_t uart, char data)
{
    USIC_CH_TypeDef *dev = 0;

    switch (uart) {
#if UART_0_EN
        case UART_0:
            dev = UART_0_DEV;
            break;
#endif
    }

    while (!(dev->PSR & 0x00000001));
    dev->TBUF[0] = (uint8_t)data;

    return 1;
}

void uart_poweron(uart_t uart)
{
    switch (uart) {
#if UART_0_EN
        case UART_0:
            UART_0_DEV->CCR |= 0x2;
            break;
#endif
    }
}

void uart_poweroff(uart_t uart)
{
    switch (uart) {
#if UART_0_EN
        case UART_0:
            UART_0_DEV->CCR &= ~(0x0000000f);
            break;
#endif
    }
}

#if UART_0_EN
void UART_0_ISR(void)
{
    irq_handler(UART_0, UART_0_DEV);
}
#endif


static inline void irq_handler(uint8_t uartnum, USIC_CH_TypeDef *dev)
{
    if (dev->PSR & 0x00000080) {
        char data = (char)dev->RBUF;
        uart_config[uartnum].rx_cb(uart_config[uartnum].arg, data);
    }
    else if (dev->PSR & 0x00000100) {
        if (uart_config[uartnum].tx_cb(uart_config[uartnum].arg) == 0) {
            dev->PCR &= ~(0x00000080);
        }
    }
    if (sched_context_switch_request) {
        thread_yield();
    }
}

#endif /* UART_NUMOF */
