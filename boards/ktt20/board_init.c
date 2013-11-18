/*
 * board_init.c - ktt20 initialization code
 * Copyright (C) 2013 Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * This source code is licensed under the GNU Lesser General Public License,
 * Version 2.  See the file LICENSE for more details.
 */

void board_init(void)
{
    asm("nop");
}

void bl_init_clks(void)
{
    // dummy to compile
}

void bl_init_ports(void)
{
    // dummy to compile
}

int bl_uart_init(void)
{
    //
}
