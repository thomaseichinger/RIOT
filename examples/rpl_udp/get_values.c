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

#include <string.h>
#include <stdio.h>
#include "vtimer.h"
#include "thread.h"
#include "rpl_udp.h"

#include "get_values.h"

#define ENABLE_DEBUG                (0)
#include "debug.h"

#define FLOAT_PRECISION             1000

/* message buffer */
#define GET_VALUES_MSG_BUFFER_SIZE  4
msg_t msg_buffer[GET_VALUES_MSG_BUFFER_SIZE];
char get_values_stack_buffer[KERNEL_CONF_STACKSIZE_MAIN];
kernel_pid_t get_values_pid;

char get_values_stack_buffer[KERNEL_CONF_STACKSIZE_MAIN];
kernel_pid_t get_values_pid;


static void *get_values(void *);

void get_values_init(void)
{
    get_values_pid = thread_create(get_values_stack_buffer,
                                     sizeof(get_values_stack_buffer),
                                     PRIORITY_MAIN - 2,
                                     CREATE_STACKTEST,
                                     get_values,
                                     NULL,
                                     "get_values");
    printf("GET_VALUES (THREAD PID: %" PRIkernel_pid ")\n", get_values_pid);
}

static void *get_values(void *arg)
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
    return NULL;
}

void send_msg_get_values(msg_t *m){
    msg_try_send(m, get_values_pid);
}
