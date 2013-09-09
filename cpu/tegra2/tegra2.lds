/*
 * tegra2.lds - Linker script for Tegra2 devices
 * Copyright (C) 2013 Thomas Eichinger <thomas.eichinger@fu-berlin.de
 *
 * This source code is licensed under the GNU Lesser General Public License,
 * Version 2.  See the file LICENSE for more details.
 *
 * This file is part of RIOT.
 *
 */
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_startup)
SECTIONS
{
    . = 0x00000000;

    . = ALIGN(4);
    .text :
    {
        *(.__image_copy_start)
        startup.o (.text*)
        *(.text*)
    }

    . = ALIGN(4);
    .rodata : 
    {
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
    }

    . = ALIGN(4);
    .data :
    {
        *(.data*)
    }

    . = ALIGN(4);
    . = .;

    . = ALIGN(4);
    .image_copy_end :
    {
        *(.__image_copy_end)
    }

    .rel.dyn :
    {
        __rel_dyn_start = .;
        *(.rel*)
        __rel_dyn_end = .;
    }

    .dynsym :
    {
        __dynsym_start = .;
        *(.dynsym)
    }

    _end = .;

    .bss_start __rel_dyn_start (OVERLAY) :
    {
        KEEP(*(.__bss_start));
        __bss_base = .;
    }

    .bss __bss_base (OVERLAY) :
    {
        *(.bss*)
        . = ALIGN(4);
        __bss_limit = .;
    }

    .bss_end __bss_limit (OVERLAY) :
    {
        KEEP(*(.__bssend));
    }

    /DISCARD/ : { *(.dynstr*) }
    /DISCARD/ : { *(.dynamic*) }
    /DISCARD/ : { *(.plt*) }
    /DISCARD/ : { *(.interp*) }
    /DISCARD/ : { *(.gnu*) }
}