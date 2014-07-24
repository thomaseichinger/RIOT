/*
 * Copyright (C) 2014 Oliver Hahm <oliver.hahm@inria.fr>
 *
 * This file subject to the terms and conditions of the GNU Lesser General
 * Public License. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file        main.c
 *
 * @brief       Sniffer application for MSB-A2 and Wireshark
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "posix_io.h"
#include "shell.h"
#include "shell_commands.h"
#include "board_uart0.h"
#include "hwtimer.h"
#include "transceiver.h"

#define RCV_BUFFER_SIZE     (64)
#define RADIO_STACK_SIZE    (KERNEL_CONF_STACKSIZE_MAIN)
#define PER_ROW             (16)

char radio_stack_buffer[RADIO_STACK_SIZE];
msg_t msg_q[RCV_BUFFER_SIZE];
transceiver_command_t tcmd;

void print_packet(volatile radio_packet_t *p)
{
    volatile uint8_t i, j, k;

    if (p) {
        printf("len 0x%02x lqi 0x%02x rx_time 0x%08lx", p->length, p->lqi, hwtimer_now());

        for (j = 0, k = 0; j <= ((p->length) / PER_ROW); j++) {
            printf("\n\r");

            for (i = 0; i < PER_ROW; i++, k++) {
                if (k >= p->length) {
                    printf("\n\r");
                    return;
                }

                printf("%02x ", p->data[j * PER_ROW + i]);
            }
        }
    }

    printf("\n\r");
    return;
}

void radio(void)
{
    msg_t m;
    radio_packet_t *p;

    msg_init_queue(msg_q, RCV_BUFFER_SIZE);

    while (1) {
        msg_receive(&m);

        if (m.type == PKT_PENDING) {
            p = (radio_packet_t *) m.content.ptr;
            printf("rftest-rx --- ");
            print_packet(p);
            p->processing--;
        }
        else if (m.type == ENOBUFFER) {
            puts("Transceiver buffer full");
        }
        else {
            puts("Unknown packet received");
        }
    }
}

void init_transceiver(void)
{
    int radio_pid = thread_create(radio_stack_buffer, RADIO_STACK_SIZE, 
                                  PRIORITY_MAIN - 2, CREATE_STACKTEST, radio,
                                  "radio");

    uint16_t transceivers = 0;
#ifdef MODULE_CC110X
    transceivers |= TRANSCEIVER_CC1100;
#endif
#ifdef MODULE_CC110X_NG
    transceivers |= TRANSCEIVER_CC1100;
#endif
#ifdef MODULE_CC2420
    transceivers |= TRANSCEIVER_CC2420;
#endif
#ifdef MODULE_NATIVENET
    transceivers |= TRANSCEIVER_NATIVE;
#endif
#ifdef MODULE_AT86RF231
    transceivers |= TRANSCEIVER_AT86RF231;
#endif
#ifdef MODULE_MC1322X
    transceivers |= TRANSCEIVER_MC1322X;
#endif

    transceiver_init(transceivers);
    (void) transceiver_start();
    transceiver_register(transceivers, radio_pid);

    msg_t mesg;
    mesg.type = SET_CHANNEL;
    mesg.content.ptr = (char *) &tcmd;

    uint16_t c = 10;

    tcmd.transceivers = TRANSCEIVER_CC1100;
    tcmd.data = &c;
    printf("Set transceiver to channel %u\n", c);
    msg_send(&mesg, transceiver_pid, 1);

    mesg.type = SET_MONITOR;
    mesg.content.ptr = (char *) &tcmd;

    uint16_t v = 1;

    tcmd.transceivers = TRANSCEIVER_CC1100;
    tcmd.data = &v;
    printf("Set transceiver into monitor mode\n");
    msg_send(&mesg, transceiver_pid, 1);
}

static int shell_readc(void)
{
    char c = 0;
    (void) posix_read(uart0_handler_pid, &c, 1);
    return c;
}

static void shell_putchar(int c)
{
    (void) putchar(c);
}

int main(void)
{
    shell_t shell;
    (void) posix_open(uart0_handler_pid, 0);
    init_transceiver();

    (void) puts("Welcome to RIOT!");

    shell_init(&shell, NULL, UART0_BUFSIZE, shell_readc, shell_putchar);
    shell_run(&shell);

    return 0;
}
