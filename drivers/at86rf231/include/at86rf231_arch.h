#ifndef AT86RF231_ARCH_H_
#define AT86RF231_ARCH_H_

#include <stdint.h>

#include "vtimer.h"

void at86rf231_arch_init(void);

void at86rf231_arch_reset(void);
uint8_t at86rf231_arch_get_status(void);

void at86rf231_arch_spi_select(void);
void at86rf231_arch_spi_unselect(void);

void at86rf231_arch_init_interrupts(void);
void at86rf231_arch_enable_interrupts(void);
void at86rf231_arch_disable_interrupts(void);
#endif
