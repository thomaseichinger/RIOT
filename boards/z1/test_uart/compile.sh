#!/bin/bash

msp430-gcc -c -mmcu=msp430f2617 -Wall test_z1_uart.c
msp430-gcc -mmcu=msp430f2617 -o test_z1_uart.elf test_z1_uart.o
msp430-size test_z1_uart.elf
msp430-objcopy -O ihex test_z1_uart.elf test_z1_uart.hex

