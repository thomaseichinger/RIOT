/*
 * isr.c - mc1322x specific isr
 * Copyright (C) 2013 Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * This source code is licensed under the GNU Lesser General Public License,
 * Version 2.  See the file LICENSE for more details.
 *
 * This file is part of RIOT.
 */

#include "mc1322x.h"

#define MAX_IRQ_INDEX 10
#define MAX_IRQ_NUM 11

static void (*isr_funcs[11])(void) = {
    asm_isr,
    uart1_isr,
    uart2_isr,
    crm_isr,
    i2c_isr,
    tmr_isr,
    spif_isr,
    maca_isr,
    ssi_isr,
    adc_isr,
    spi_isr
};

void register_isr(uint8_t interrupt, void (*isr)(void)) {
    if (interrupt <= MAX_IRQ_INDEX) {
        isr_funcs[interrupt] = isr;
    }
}

void isr(void)
{
    /* set irq_num to an invalid value */
    uint8_t irq_num = MAX_IRQ_NUM;

    /* pending interrupt? */
    while (ITC->NIPEND) {
        /* get interrupt source, range 0-10 */
        irq_num = ITC->NIVECTOR;
        /* call corresponding ISR */
        if (isr_funcs[irq_num] != 0) {
            (isr_funcs[irq_num])();
        }
    }
}
