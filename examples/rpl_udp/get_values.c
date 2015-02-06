/*
 * Copyright (C)
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
 * @brief
 *
 * @author
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "vtimer.h"
#include "thread.h"
#include "udp.h"
#include "rpl_udp.h"

#include "get_values.h"

#define ENABLE_DEBUG                (0)
#include "debug.h"

#define FLOAT_PRECISION             1000

/* message buffer */
#define GET_VALUES_MSG_BUFFER_SIZE  4
msg_t msg_buffer[GET_VALUES_MSG_BUFFER_SIZE];

void *get_values(void *arg)
{
    timex_t sleep = timex_set(1, 0);
    msg_init_queue(msg_buffer, GET_VALUES_MSG_BUFFER_SIZE);
    msg_t m;
    uint8_t is_alive = 1;

    while (1) {
        if(msg_try_receive(&m) == 1){
            if(strcmp(m.content.ptr,"start") == 0){
                printf("start received\n");
                is_alive = 1;
            }
            else if(strcmp(m.content.ptr,"stop") == 0){
                printf("stop received\n");
                is_alive = 0;
            }
        }
        vtimer_sleep(sleep);
        if(is_alive){
            char msg[30];

            sprintf(msg,"%d|20,366|38,894|20,458|40,254",
                        ADDRESS_ROOM);

            /* A configurer selon la carte */
            /*Pour les noeuds: */
            udp_send(ADDRESS_PASSERELLE,msg);

            /*Pour les cartes connectées au pc: */
            puts(msg);
        }

    }
}
