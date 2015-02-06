/*
 * Copyright (C) 2013, 2014 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup examples
 * @{
 *
 * @file
 * @brief UDP RPL example application
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "vtimer.h"
#include "thread.h"
#include "net_if.h"
#include "sixlowpan.h"
#include "udp.h"
#include "rpl.h"
#include "rpl/rpl_dodag.h"
#include "rpl_udp.h"
#include "transceiver.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define TRANSCEIVER TRANSCEIVER_DEFAULT

radio_address_t id;

uint8_t is_root = 0;

void rpl_udp_init(void)
{
    transceiver_command_t tcmd;
    msg_t m;
    uint32_t chan = RADIO_CHANNEL;

    net_if_set_hardware_address(0, id);

    /* add global address */
    ipv6_addr_t tmp;
    /* initialize prefix */
    ipv6_addr_init(&tmp, 0xabcd, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, id);
    /* set host suffix */
    ipv6_addr_set_by_eui64(&tmp, 0, &tmp);
    ipv6_net_if_add_addr(0, &tmp, NDP_ADDR_STATE_PREFERRED, 0, 0, 0);

    /* set channel to 12 */
    tcmd.transceivers = TRANSCEIVER;
    tcmd.data = &chan;
    m.type = SET_CHANNEL;
    m.content.ptr = (void *) &tcmd;

    msg_send_receive(&m, &m, transceiver_pid);
    printf("Channel set to %u\n", RADIO_CHANNEL);

    puts("Transport layer initialized");
    /* start transceiver watchdog */
}
