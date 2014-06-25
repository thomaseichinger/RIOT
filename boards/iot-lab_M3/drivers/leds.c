#include <stdio.h>
#include <stddef.h>

#include "stm32f10x.h"

void leds_init(void)
{
    /* green pin */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL = (0x3 << (5*4));

    /* orange pin */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH = (0x3 << ((10-8)*4));

    /* red pin */
    RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
    GPIOD->CRL = (0x3 << (2*4));
}
