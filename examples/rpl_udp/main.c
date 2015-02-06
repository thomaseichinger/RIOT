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
#include <math.h>

#include "net_if.h"
#include "posix_io.h"
#include "board_uart0.h"
#include "udp.h"
#include "periph_conf.h"
#include "get_values.h"
#include "periph/timer.h"
#include "periph/gpio.h"
#include "thread.h"

#include "rpl_udp.h"


void udp_rpl_init(void){
    net_if_set_src_address_mode(0, NET_IF_TRANS_ADDR_M_SHORT);
    id = net_if_get_hardware_address(0);

    //initialisation du rpl et du udp
    rpl_udp_set_id(ADDRESS_IPV6_BOARD);
    rpl_udp_init();
    udp_server();
}

int main(void)
{
    puts("RPL router v"APP_VERSION);

    int value = 0;

    udp_rpl_init();

#if NODE
    get_values_pid = thread_create(get_values_stack_buffer,
                                     sizeof(get_values_stack_buffer),
                                     PRIORITY_MAIN - 2,
                                     CREATE_STACKTEST,
                                     get_values,
                                     NULL,
                                     "get_values");
#endif /* NODE */

    return 0;
}
