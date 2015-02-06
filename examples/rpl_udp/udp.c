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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "thread.h"

#include "socket_base/socket.h"

#include "net_help.h"

#include "rpl_udp.h"

#include "get_values.h"

#define UDP_BUFFER_SIZE     (128)
#define SERVER_PORT     (0xFF01)

char udp_server_stack_buffer[KERNEL_CONF_STACKSIZE_MAIN];
char addr_str[IPV6_MAX_ADDR_STR_LEN];

static void *init_udp_server(void *);

/* UDP server thread */
void udp_server(void)
{
    kernel_pid_t udp_server_thread_pid = thread_create(udp_server_stack_buffer,
                                                       sizeof(udp_server_stack_buffer),
                                                       PRIORITY_MAIN, CREATE_STACKTEST,
                                                       init_udp_server,
                                                       NULL,
                                                       "init_udp_server");
    printf("UDP SERVER ON PORT %d (THREAD PID: %" PRIkernel_pid ")\n", HTONS(SERVER_PORT), udp_server_thread_pid);
}

static void *init_udp_server(void *arg)
{
    (void) arg;
    
    sockaddr6_t sa;
    char buffer_main[UDP_BUFFER_SIZE];
    uint32_t fromlen;
    int sock = socket_base_socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    memset(&sa, 0, sizeof(sa));

    sa.sin6_family = AF_INET;
    sa.sin6_port = HTONS(SERVER_PORT);

    fromlen = sizeof(sa);

    if (-1 == socket_base_bind(sock, &sa, sizeof(sa))) {
        printf("Error bind failed!\n");
        socket_base_close(sock);
        return NULL;
    }

    msg_t m;

    while (1) {
        int32_t recsize = socket_base_recvfrom(sock, (void *)buffer_main, UDP_BUFFER_SIZE, 0, &sa, &fromlen);

        if (recsize < 0) {
            printf("ERROR: recsize < 0!\n");
        }

        //TODO check that get_values_thread is running
        if(strcmp(buffer_main,"start") == 0 || strcmp(buffer_main,"stop") == 0 ){
            m.type = 0;
            m.content.ptr = (void *) buffer_main;
            msg_try_send(&m, get_values_pid);
        }
        else{
//        printf("UDP packet received, payload: %s\n", buffer_main);
            printf("%s\n",buffer_main);
        }
    }

    socket_base_close(sock);

    return NULL;
}

/* UDP send command */
void udp_send(int add,char *c)
{
    int sock;
    sockaddr6_t sa;
    ipv6_addr_t ipaddr;
    int bytes_sent;
    int address;
    char text[35];

    address = add;

    strncpy(text, c, sizeof(text));
    text[sizeof(text) - 1] = 0;

    sock = socket_base_socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (-1 == sock) {
        printf("Error Creating Socket!");
        return;
    }

    memset(&sa, 0, sizeof(sa));

    if (address) {
        ipv6_addr_init(&ipaddr, 0xabcd, 0x0, 0x0, 0x0, 0x0, 0x00ff, 0xfe00, (uint16_t)address);
    }
    else {
        ipv6_addr_set_all_nodes_addr(&ipaddr);
    }

    sa.sin6_family = AF_INET;
    memcpy(&sa.sin6_addr, &ipaddr, 16);
    sa.sin6_port = HTONS(SERVER_PORT);

    //ipv6_iface_set_routing_provider(&ipaddr);

    bytes_sent = socket_base_sendto(sock, (char *)text,
                                       strlen(text) + 1, 0, &sa,
                                       sizeof(sa));

    if (bytes_sent < 0) {
        printf("Error sending packet!\n");
    }
    else {
        printf("Successful deliverd %i bytes over UDP to %s to 6LoWPAN\n",
               bytes_sent, ipv6_addr_to_str(addr_str, IPV6_MAX_ADDR_STR_LEN,
                                            &ipaddr));
    }

    socket_base_close(sock);
}
    
