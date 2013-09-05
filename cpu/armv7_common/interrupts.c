/*
 * interrupts.c - Inerrupt service routines
 * Copyright (C) 2013 Thomas Eichinger <thomas.eichinger@fu-berlin.de
 *
 * This source code is licensed under the GNU Lesser General Public License,
 * Version 2.  See the file LICENSE for more details.
 *
 * This file is part of RIOT.
 *
 */

/*
 * TODO: unlock this when we have a gd inrstructure
 */
/*
uint32_t interrupt_init(void)
{
	IRQ_STACK_START = gd->irq_sp - 4;
	IRQ_STACK_START_IN = gd->irq_sp + 8;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;

	return arch_interrupt_init();
}
*/

/*
 * Enables IRQ interrupts
 */
void enable_interrupts(void) 
{
	uint32_t tmp;
	__asm__ __volatile__(	"mrs %0, cpsr\n"
							"bic %0, %0, #0x80\n"
							"msr cpsr_c, %0"
							: "=r" (tmp)
							:
							: "memory");
}

/*
 * Disables IRQ interrupts
 * \return true if interrupts had already been disabled
 */
uint32_t disable_interrupts(void)
{
	uint32_t tmp;
	uint32_t bck;
	__asm__ __volatile__(	"mrs %0, cpsr\n"
							"orr %1, %0, #0xc0\n"
							"msr cpsr_c, %1"
							: "=r" (bck), "=r" (tmp)
							:
							: "memory");
	return !(bck & 0x80);
}

void bad_mode(void) 
{
	printf("Resetting CPU ...\n");
	reset_cpu(0); 
}

void handle_undefined_instruction(void)
{
	printf("undefined_instruction\n");
	bad_mode();
}

void handle_software_interrupt(void)
{
	printf("software_interrupt\n");
	bad_mode();
}

void handle_prefetch_abort(void)
{
	printf("prefetch_abort\n");
	bad_mode();
}

void handle_data_abort(void)
{
	printf("data_abort\n");
	bad_mode();
}

void handle_not_used(void)
{
	printf("not_used\n");
	bad_mode();
}

/*
 * TODO: handle_irq, handle_fiq
 */
