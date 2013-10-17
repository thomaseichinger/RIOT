/*
 * startup.s - mc1322x specific startup code
 * Copyright (C) 2013 Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * This source code is licensed under the GNU Lesser General Public License,
 * Version 2.  See the file LICENSE for more details.
 *
 * This file is part of RIOT.
 */
    
/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs (program status registers) */
.set  USR_MODE, 0x10                    /* Normal User Mode                                         */
.set  FIQ_MODE, 0x11                    /* FIQ Processing Fast Interrupts Mode                      */
.set  IRQ_MODE, 0x12                    /* IRQ Processing Standard Interrupts Mode                  */
.set  SVC_MODE, 0x13                    /* Supervisor Processing Software Interrupts Mode           */
.set  ABT_MODE, 0x17                    /* Abort Processing memory Faults Mode                      */
.set  UND_MODE, 0x1B                    /* Undefined Processing Undefined Instructions Mode         */
.set  SYS_MODE, 0x1F                    /* System Running Priviledged Operating System Tasks  Mode  */

.set  IRQ_DISABLE, 0x80                     /* when I bit is set, IRQ is disabled (program status registers) */
.set  FIQ_DISABLE, 0x40                     /* when F bit is set, FIQ is disabled (program status registers) */


       .section .startup
    
       .set _rom_data_init, 0x108d0
       .global _startup
       .func _startup

_startup:
        b     _begin                    /* reset - _start */
        ldr     PC, Undef_Addr      /* Undefined Instruction */
        ldr     PC, SWI_Addr        /* Software Interrupt */
        ldr     PC, PAbt_Addr       /* Prefetch Abort */
        ldr     PC, DAbt_Addr       /* Data Abort */
        ldr     PC, _not_used
        ldr     PC, IRQ_Addr        /* Interrupt Request Interrupt (load from VIC) */
        ldr     PC, _fiq

 /* these vectors are used for rom patching */  
.org 0x20
.code 16
_RPTV_0_START:
    bx lr /* do nothing */

.org 0x60
_RPTV_1_START:
    bx lr /* do nothing */

.org 0xa0
_RPTV_2_START:
    bx lr /* do nothing */

.org 0xe0
_RPTV_3_START:
    bx lr /* do nothing */

.org 0x120
ROM_var_start: .word 0
.org 0x7ff
ROM_var_end:   .word 0

.code 32
.align          
_begin: 
    /* FIQ mode stack */
    msr CPSR_c, #(FIQ_MODE | IRQ_DISABLE | FIQ_DISABLE)
    ldr sp, =__fiq_stack_top__  /* set the FIQ stack pointer */

    /* IRQ mode stack */
    msr CPSR_c, #(IRQ_MODE | IRQ_DISABLE | FIQ_DISABLE)
    ldr sp, =__irq_stack_top__  /* set the IRQ stack pointer */

    /* Supervisor mode stack */
    msr CPSR_c, #(SVC_MODE | IRQ_DISABLE | FIQ_DISABLE)
    ldr sp, =__svc_stack_top__  /* set the SVC stack pointer */

    /* Undefined mode stack */
    msr CPSR_c, #(UND_MODE | IRQ_DISABLE | FIQ_DISABLE)
    ldr sp, =__und_stack_top__  /* set the UND stack pointer */

    /* Abort mode stack */
    msr CPSR_c, #(ABT_MODE | IRQ_DISABLE | FIQ_DISABLE)
    ldr sp, =__abt_stack_top__  /* set the ABT stack pointer */

    /* System mode stack */
    msr CPSR_c, #(SYS_MODE | IRQ_DISABLE | FIQ_DISABLE)
    ldr sp, =__sys_stack_top__  /* set the SYS stack pointer */

    /* call the rom_data_init function in ROM */
    /* initializes ROM_var space defined by ROM_var_start and ROM_var_end */
    /* this area is used by ROM functions (e.g. nvm_read) */
    ldr r12,=_rom_data_init
    mov lr,pc
    bx  r12

    msr CPSR_c, #(SYS_MODE)

        /* Clear BSS */
clear_bss:
        ldr     r0, _bss_start          /* find start of bss segment        */
        ldr     r1, _bss_end            /* stop here                        */
        mov     r2, #0x00000000         /* clear                            */
clbss_l:
        str     r2, [r0]                /* clear loop...                    */
        add     r0, r0, #4
        cmp     r0, r1
        blt     clbss_l

        bl bootloader
        b  kernel_init

/* Exception vector handlers branching table */
Undef_Addr:     .word   UNDEF_Routine       
SWI_Addr:       .word   ctx_switch          
PAbt_Addr:      .word   PABT_Routine        
DAbt_Addr:      .word   DABT_Routine        
_not_used:      .word   not_used
IRQ_Addr:       .word   irq                 
_fiq:           .word   fiq
    .balignl    16, 0xdeadbeef

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
        .word __bss_start

    .globl _bss_end
_bss_end:
        .word _end

.align  5
not_used:

    .align  5
/*irq:
//
//  .align  5*/
fiq:

    .align  5
