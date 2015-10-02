/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Demonstrating the sending and receiving over MS/TP
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <inttypes.h>

#include "kernel.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/pktdump.h"
#include "timex.h"
#include "xtimer.h"

int mstp_cmd(int argc, char **argv)
{
    // if (argc < 2) {
    //     printf("usage: %s [send|server]\n", argv[0]);
    //     return 1;
    // }

    // if (strcmp(argv[1], "send") == 0) {
    //     uint32_t num = 1;
    //     uint32_t delay = 1000000;
    //     if (argc < 5) {
    //         printf("usage: %s send <addr> <data> [<num> [<delay in us>]]\n",
    //                argv[0]);
    //         return 1;
    //     }
    //     if (argc > 4) {
    //         num = (uint32_t)atoi(argv[4]);
    //     }
    //     if (argc > 5) {
    //         delay = (uint32_t)atoi(argv[5]);
    //     }
    //     send(argv[2], argv[3], num, delay);
    // }
    // else if (strcmp(argv[1], "server") == 0) {
    //     if (argc < 3) {
    //         printf("usage: %s server [start|stop]\n", argv[0]);
    //         return 1;
    //     }
    //     if (strcmp(argv[2], "start") == 0) {
    //         start_server();
    //     }
    //     else if (strcmp(argv[2], "stop") == 0) {
    //         stop_server();
    //     }
    //     else {
    //         puts("error: invalid command");
    //     }
    // }
    // else {
    //     puts("error: invalid command");
    // }
    return 0;
}
