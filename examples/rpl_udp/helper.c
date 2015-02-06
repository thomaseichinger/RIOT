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
#include <stdlib.h>
#include <string.h>
#include "msg.h"
#include "sixlowpan/ip.h"
#include "transceiver.h"
#include "ieee802154_frame.h"
#include "rpl/rpl_structs.h"

#include "rpl_udp.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define LL_HDR_LEN  (0x4)
#define IPV6_HDR_LEN    (0x28)

extern uint8_t ipv6_ext_hdr_len;

//static msg_t msg_q[RCV_BUFFER_SIZE];

void rpl_udp_set_id(int i)
{
    id = i;

    printf("Set node ID to %u\n", id);
}
