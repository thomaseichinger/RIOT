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
 * @brief       Implementation of the CPU initialization
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @}
 */

#include <stdint.h>
#include "cpu.h"
#include "periph_conf.h"

/**
 * @name Pattern to write into the Coprocessor Access Control Register to allow full FPU access
 */
#define FULL_FPU_ACCESS         (0x00f00000)

#define CLOCK_FSYS              (120000000)
#define CLOCK_CRYSTAL_FREQUENCY (12000000)
#define SCU_OSC_HP_MODE         (0xF0)
#define SCU_CPUCLKCR_DIV        (0x00000000)    /* no prescaler */
#define SCU_PBCLKCR_DIV         (0x00000000)    /* no prescaler */
#define SCU_CCUCLKCR_DIV        (0x00000000)    /* no prescaler */

#define     SCU_PLL_K1DIV   1
#define     SCU_PLL_K2DIV   3
#define     SCU_PLL_PDIV    1
#define     SCU_PLL_NDIV    79



static void cpu_clock_init(void);

/**
 * @brief Initialize the CPU, set IRQ priorities
 */
void cpu_init(void)
{
    /* give full access to the FPU */
    SCB->CPACR |= (uint32_t)FULL_FPU_ACCESS;

    /* setup flash wait state */
    FLASH0->FCON |= (~FLASH_FCON_WSPFLASH_Msk | 0x3);

    /* configure the vector table location to internal flash */
    // SCB->VTOR = FLASH_BASE;

    /* initialize the clock system */
    cpu_clock_init();

    /* disable watchdog timer */
    WDT->CTR &= ~WDT_CTR_ENB_Msk;

    /* set pendSV interrupt to lowest possible priority */
    NVIC_SetPriority(PendSV_IRQn, 0xff);
}

/**
 * @brief Configure the controllers clock system
 *
 * The clock initialization make the following assumptions:
 * - the external HSE clock from an external oscillator is used as base clock
 * - the internal PLL circuit is used for clock refinement
 *
 * Use the following formulas to calculate the needed values:
 *
 * SYSCLK = ((HSE_VALUE / CLOCK_PLL_M) * CLOCK_PLL_N) / CLOCK_PLL_P
 * USB, SDIO and RNG Clock =  ((HSE_VALUE / CLOCK_PLL_M) * CLOCK_PLL_N) / CLOCK_PLL_Q
 *
 * The actual used values are specified in the board's `periph_conf.h` file.
 *
 * NOTE: currently there is not timeout for initialization of PLL and other locks
 *       -> when wrong values are chosen, the initialization could stall
 */
static void cpu_clock_init(void)
{
    uint32_t vco;
    uint32_t stepping_K2DIV;

    /* check if PLL is switched on */
    if ((SCU_PLL->PLLCON0 &(SCU_PLL_PLLCON0_VCOPWD_Msk | SCU_PLL_PLLCON0_PLLPWD_Msk)) != 0){
        /* enable PLL first */
        SCU_PLL->PLLCON0 &= ~(SCU_PLL_PLLCON0_VCOPWD_Msk | SCU_PLL_PLLCON0_PLLPWD_Msk);
    }

    /*enable the OSC_HP*/
    SCU_OSC->OSCHPCTRL &= ~(SCU_OSC_HP_MODE);

    /* select external OSC as PLL input */
    SCU_PLL->PLLCON2 &= ~SCU_PLL_PLLCON2_PINSEL_Msk;
    
    while(((SCU_PLL->PLLSTAT) 
          & (SCU_PLL_PLLSTAT_PLLHV_Msk 
             | SCU_PLL_PLLSTAT_PLLLV_Msk 
             |SCU_PLL_PLLSTAT_PLLSP_Msk)) != 0x380);

    /* select FOFI as system clock */
    if((SCU_CLK->SYSCLKCR & SCU_CLK_SYSCLKCR_SYSSEL_Msk) != 0x0) {
        /*Select FOFI*/
        SCU_CLK->SYSCLKCR &= ~SCU_CLK_SYSCLKCR_SYSSEL_Msk;
    }

    vco = (CLOCK_CRYSTAL_FREQUENCY/(SCU_PLL_PDIV+1))*(SCU_PLL_NDIV+1);
    stepping_K2DIV = (vco/24000000)-1;

    /* Go to bypass the Main PLL */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_VCOBYP_Msk;
    /* disconnect OSC_HP to PLL */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_FINDIS_Msk;
    /* Setup devider settings for main PLL */
    SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | (stepping_K2DIV<<16) | (SCU_PLL_PDIV<<24));
    /* we may have to set OSCDISCDIS */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_OSCDISCDIS_Msk;
    /* connect OSC_HP to PLL */
    SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_FINDIS_Msk;
    /* restart PLL Lock detection */
    SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_RESLD_Msk;

    /*********************************************************
    here we need to setup the system clock divider
    *********************************************************/

    SCU_CLK->CPUCLKCR = SCU_CPUCLKCR_DIV;
    SCU_CLK->PBCLKCR = SCU_PBCLKCR_DIV; 
    SCU_CLK->CCUCLKCR = SCU_CCUCLKCR_DIV;


    /* Switch system clock to PLL */
    SCU_CLK->SYSCLKCR |=  0x00010000; 
            
    /* we may have to reset OSCDISCDIS */
    SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCDISCDIS_Msk;

    /* ramping up PLL to 120MHz*/
    /* calulation for stepping */
    stepping_K2DIV = (vco/60000000)-1;
    /* Setup devider settings for main PLL */
    SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | (stepping_K2DIV<<16) | (SCU_PLL_PDIV<<24));

    stepping_K2DIV = (vco/90000000)-1;         
    /* Setup devider settings for main PLL */
    SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | (stepping_K2DIV<<16) | (SCU_PLL_PDIV<<24));

    /* Setup devider settings for main PLL */
    SCU_PLL->PLLCON1 = ((SCU_PLL_K1DIV) | (SCU_PLL_NDIV<<8) | (SCU_PLL_K2DIV<<16) | (SCU_PLL_PDIV<<24));
    
    SCU_TRAP->TRAPCLR = SCU_TRAP_TRAPCLR_SOSCWDGT_Msk | SCU_TRAP_TRAPCLR_SVCOLCKT_Msk;  /* clear request for System OCS Watchdog Trap and System VCO Lock Trap  */
}
