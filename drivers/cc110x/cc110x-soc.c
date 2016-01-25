/*
 * Copyright (C) 2016 FU Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_cc110x
 * @{
 *
 * @file
 * @brief       TI Chipcon CC110x SoC driver
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @}
 */

#include <stdio.h>

#include "cc110x.h"
#include "cc110x-spi.h"
#include "cc110x-internal.h"
#include "cc110x-defines.h"

#include "periph/gpio.h"

#include "xtimer.h"
#include "irq.h"

#if RIOT_CPU == CPU_CC430

void cc110x_writeburst_reg(cc110x_t *dev, uint8_t addr, const char *src, uint8_t count)
{
    unsigned int cpsr;
    cpsr = disableIRQ();

    while (!(RF1AIFCTL1 & RFINSTRIFG));
    RF1AINSTRW = ((addr | RF_REGWR)<<8) + src[0];

    for (uint8_t i = 0; i < count; i++) {
        while (!(RF1AIFCTL1 & RFINSTRIFG));
        RF1ADINB = src[i];
    }

    restoreIRQ(cpsr);
}

void cc110x_readburst_reg(cc110x_t *dev, uint8_t addr, char *buffer, uint8_t count)
{
    unsigned int cpsr;
    cpsr = disableIRQ();

    while (!(RF1AIFCTL1 & RFINSTRIFG));
    RF1AINSTR1B = (addr | RF_REGRD);

    for (uint8_t i = 0; i < (count-1); i++) {
        while (!(RF1AIFCTL1 & RFDOUTIFG));
        buffer[i] = RF1ADOUT1B;
    }

    buffer[count-1] = RF1ADOUT0B;

    restoreIRQ(cpsr);
}

void cc110x_write_reg(cc110x_t *dev, uint8_t addr, uint8_t value)
{
    unsigned int cpsr;
    cpsr = disableIRQ();

    while (!(RF1AIFCTL1 & RFINSTRIFG));
    RF1AINSTRB = (addr | RF_SNGLREGWR);

    RF1ADINB = value;

    restoreIRQ(cpsr);
}

uint8_t cc110x_read_reg(cc110x_t *dev, uint8_t addr)
{
    uint8_t result;
    unsigned int cpsr;
    cpsr = disableIRQ();

    /* Check for valid configuration register address, 0x3E refers to PATABLE */
    if ((addr <= 0x2E) || (addr == 0x3E)) {
        RF1AINSTR1B = (addr | RF_SNGLREGRD);
    }
    else {
        RF1AINSTR1B = (addr | RF_STATREGRD);
    }

    while (!(RF1AIFCTL1 & RFDOUTIFG));
    result = RF1ADOUTB; 

    restoreIRQ(cpsr);
    return result;
}

uint8_t cc110x_read_status(cc110x_t *dev, uint8_t addr)
{
    return cc110x_read_reg(dev, addr);
}

uint8_t cc110x_get_reg_robust(cc110x_t *dev, uint8_t addr)
{
    uint8_t result, result2;

    do {
        result = cc110x_read_reg(dev, addr);
        result2 = cc110x_read_reg(dev, addr);
    } while (result != result2);
    
    return (uint8_t) result;
}

uint8_t cc110x_strobe(cc110x_t *dev, uint8_t c)
{
#ifdef CC110X_DONT_RESET
    if (c == CC110X_SRES) {
        return 0;
    }
#endif

    uint8_t result;
    uint16_t gdo_state;
    unsigned int cpsr;
    cpsr = disableIRQ();

    /* Check for valid strobe command */
    if((c == 0xBD) || ((c >= RF_SRES) && (c <= RF_SNOP))) {
        RF1AIFCTL1 &= ~(RFSTATIFG);    
        while( !(RF1AIFCTL1 & RFINSTRIFG));

        if ((c > RF_SRES) && (c < RF_SNOP)) {
            gdo_state = cc110x_read_reg(dev, IOCFG2);
            cc110x_write_reg(dev, IOCFG2, 0x29);

            RF1AINSTRB = c;

            if ((RF1AIN&0x04) == 0x04) {
                if ((c == RF_SXOFF) || (c == RF_SPWD) || (c == RF_SWOR)) { 
                }
                else {
                    while ((RF1AIN&0x04) == 0x04);
                    __delay_cycles(850);              
                }
            }
            cc110x_write_reg(dev, IOCFG2, gdo_state);

            while (!(RF1AIFCTL1 & RFSTATIFG));
        }
        else { 
            RF1AINSTRB = c;     
        }
        
        result = RF1ASTATB;    
    }

    restoreIRQ(cpsr);
    return (uint8_t) result;
}

#endif /* RIOT_CPU == CPU_CC430 */