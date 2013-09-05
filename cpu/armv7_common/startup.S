/*
 * startup.S - Startup Code for ARM Cortex CPU-core
 * Copyright (C) 2013 Thomas Eichinger <thomas.eichinger@fu-berlin.de
 *
 * This source code is licensed under the GNU Lesser General Public License,
 * Version 2.  See the file LICENSE for more details.
 *
 * This file is part of RIOT.
 *
 */

#define MODE_SVC            0x13
#define CPSR_MODE_BITS      0x1f

 .global _startup
 _startup: b    reset
    ldr pc, _undefined_instruction
    ldr pc, _software_interrupt
    ldr pc, _prefetch_abort
    ldr pc, _data_abort
    ldr pc, _not_used
    ldr pc, _irq
    ldr pc, _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt: .word software_interrupt
_prefetch_abort: .word prefetch_abort
_data_abort: .word data_abort
_not_used: .word not_used
_irq: .word irq
_fiq: .word fiq
_pad: .word 0x87654321

.global _end_vect
_end_vect:
    .balignl 16, 0xdeadbeef

.global _TEXT_BASE
_TEXT_BASE:
    .word CONFIG_SYS_TEXT_BASE

.global _bss_start_ofs
_bss_start_ofs:
    .word __bss_start - _start

.global _bss_end_ofs
_bss_end_ofs:
    .word __bss_end - _start

.global _end_ofs
_end_ofs:
    .word _end - _start

.global IRQ_STACK_START
IRQ_STACK_START:
    .word 0x0badc0de

.global FIQ_STACK_START
FIQ_STACK_START:
    .word 0x0badc0de

.global IRQ_STACK_START_IN
IRQ_STACK_START_IN:
    .word 0x0badc0de

/*
 * reset code
 */
reset:
    bl save_boot_params
    /* disable FIQ and IRQ, set cpu to SVC32 mode (not if in HYP mode) */
    mrs     r0, cpsr
    and     r1, r0, #CPSR_MODE_BITS /* mask mode bits */
    teq     r1, #0x1a               /* test if HYP mode */
    bicne   r0, r0, #CPSR_MODE_BITS /* clear all mode bits */
    orrne   r0, r0, #MODE_SVC       /* set SVC mode */
    orr     r0, r0, #0xc0           /* disable FIQ and IRQ */
    msr     cpsr, r0

    bl      cpu_init_cp15
    bl      cpu_init_crit

    bl      _main

ENTRY(cpu_init_cp15)
    /* Invalidate L1 I/D-Cache */
    eor     r0, r0, r0              /* prepare r0 */
    mcr     p15, 0, r0, c8, c7, 0   /* invalidate TLBs */
    mcr     p15, 0, r0, c7, c5, 0   /* invalidate I-Cache */
    mcr     p15, 0, r0, c7, c5, 6   /* invalidate BP array */
    mcr     p15, 0, r0, c7, c10, 4  /* DSP */
    mcr     p15, 0, r0, c7, c5, 4   /* ISB */

    /* Disable MMU and chaches */
    mrc     p15, 0, r0, c1, c0, 0   /* get it */
    bic     r0, r0, #0x00002000     /* clear bit 13 */
    bic     r0, r0, #0x00000007     /* clear bits 2:0 */
    orr     r0, r0, #0x00000800     /* set bit 11 Align*/
    orr     r0, r0, #0x00000002     /* set bit 1 BTB*/
    orr     r0, r0, #0x00001000     /* set bit 12 I-Cache */
    mcr     p15, 0, r0, c1, c0, 0   /* restore it */

    /*
     * TODO: 
     * Handle ARM errata 716044, 742230, 743622, 751472 here if needed.
     */

    mov     pc, lr      /* return */

ENDPROC(cpu_init_cp15)

/* Calls lowlevel_init for board specific init */
ENTRY(cpu_init_crit)
    b       lowlevel_init           /* shall setup pll, mux, memory */
ENDPROC(cpu_init_crit)

ENTRY(c_runtime_cpu_setup)
    /* Invalidate I-Cache */
    mcr     p15, 0, r0, c7, c5, 0   /* invalidate I-cache */
    mcr     p15, 0, r0, c7, c10, 4  /* DSB */
    mcr     p15, 0, r0, c7, c5, 4   /* ISB */

    /* set vector address in cp15 VBAR reg */
    ldr     r0 = _start
    mcr     p15, 0, r0, c12, c0, 0  /* set VBAR */

    bx      lr                      /* back to caller */
ENDPROC(c_runtime_cpu_setup)

/*
 * void save_boot_params ( uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3 )
 * __attribute__((weak));
 */
ENTRY(save_boot_params)
    bx      lr                      /* back to caller */
ENDPROC(save_boot_params)
    .weak   save_boot_params

/*
 * Concerning Interrupt Handling
 */
#define STACK_FRAME_SIZE    0x48
#define STACK_PC            0x3c
#define STACK_SP            0x34

/* used for abort/prefetch/undefined/... exeptions */
.macro bad_save_user_regs
    sub     sp, sp, #STACK_FRAME_SIZE   /* get frame of current user stack */
    stmia   sp, {r0 - r12}              /* save user registerlist r0-r12 */
                                        /* in svc mode now */
    ldr     r2,  IRQ_STACK_START_IN     /* 2 words into abort stack */
    ldmia   r2, {r2 - r3}               /* get aborted pc and cpsr  */
    add     r0, sp, #STACK_FRAME_SIZE   /* get pointer to old stack */
    add     r5, sp, #STACK_SP
    mov     r1, lr 
    stmia   r5, {r0 - r3}               /* save sp_svc, lr_svc, pc, cpsr */
    mov     r0, sp                      /* copy current stack to r0 */
.endm

/* used for IRQ/FIQ handling */
.macro irq_save_user_regs
    sub     sp, sp, #STACK_FRAME_SIZE   /* get frame of current user stack */
    stmia   sp, {r0 - r12}              /* save user registers r0 - r12 */
    add     r8, sp, #STACK_PC           
    stmdb   r8, {sp, lr}^               /* save calling sp, lr */
    str     lr, [r8, #0x0]              /* save calling pc */
    mrs     r6, spsr
    str     r6, [r8, #0x4]              /* save cpsr */
    str     r0, [r8, #0x8]              /* save old r0 */
    mov     r0, sp 
.endm

.macro irq_restore_user_regs
    ldmia   sp, {r0 - lr}^              /* calling r0 - lr */
    mov     r0, r0                      /* one NOP for Pooh */
    ldr     lr, [sp, #STACK_PC]         /* get the pc */
    add     sp, sp, #STACK_FRAME_SIZE
    subs    pc, lr, #0x4                /* return and move spsr_svc into cpsr */
.endm

.macro bad_get_stack
    ldr     r13, IRQ_STACK_START_IN     /* setup mode stack (banked mode) */
    str     lr, [r13]                   /* save caller lr to saved stack */
    mrs     lr, spsr                    /* get spsr */
    str     lr, [r13, #0x4]             /* save caller spsr to saved stack */
    mov     r13, #MODE_SVC              /* prepare for SVC-mode */
    msr     spsr, r13                   /* switch to mode */
    mov     lr, pc                      /* get return pc */
    movs    pc, lr                      /* jump to next instr. and switch mode */
.endm

.macro bad_get_stack_swi
    sub     r13, r13, #0x4              /* one word of space in current stack */
    str     r0, [r13]                   /* store r0 value in stack */
    ldr     r0, IRQ_STACK_START_IN      /* get saved stack */
    str     lr, [r0]                    /* store caller lr in saved stack */
    mrs     lr, spsr                    /* get spsr */
    str     lr, [r0, #0x4]              /* store spsr in saved stack */
    ldr     lr, [r0]                    /* restore lr */
    ldr     r0, [r13]                   /* restore r0 */
    add     r13, r13, #0x4              /* restore stack pointer */
.endm

.macro irq_get_stack
    ldr     sp, IRQ_STACK_START         /* prepare IRQ stack */
.endm

.macro fiq_get_stack
    ldr     sp, FIQ_STACK_START         /* prepare FIQ stack */
.endm

/*
 * And now for something completly different.
 * exception handlers
 */
    .align 5
undefined_instruction:
    bad_get_stack
    bad_save_user_regs
    bl handle_undefined_instruction

    .align 5
software_interrupt:
    bad_get_stack_swi
    bad_save_user_regs
    bl handle_software_interrupt

    .align 5
prefetch_abort:
    bad_get_stack
    bad_save_user_regs
    bl handle_prefetch_abort

    .align 5
data_abort:
    bad_get_stack
    bad_save_user_regs
    bl  handle_data_abort

    .align 5
not_used:
    bad_get_stack
    bad_save_user_regs
    bl handle_not_used

    .align 5
irq:
    irq_get_stack
    irq_save_user_regs
    bl handle_irq
    irq_restore_user_regs

    .align 5
fiq:
    fiq_get_stack
    irq_save_user_regs              /* maybe replace with "super low latency" fiq_save_user_regs */
    bl handle_fiq
    irq_restore_user_regs