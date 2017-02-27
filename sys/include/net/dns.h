/*
 * Copyright (C) 2017 Thomas Eichinger <thomas@riot-os.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_dns    dns
 * @ingroup     net
 * @{
 *
 * @file
 * @brief       Provides simple DNS client
 *
 * @author      Thomas Eichinger <thomas@riot-os.org>
 */

#ifndef DNS_H
#define DNS_H

#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t q_count;       /**< nr of question entries  */
    uint16_t a_count;       /**< nr of answer entries    */
    uint16_t auth_count;    /**< nr of authority entries */
    uint16_t res_count;     /**< nr of resource entries  */
    uint8_t payload[];
} dns_pkt_t;

#define DNS_TYPE_A          (1)
#define DNS_TYPE_NS         (2)
#define DNS_TYPE_CNAME      (5)
#define DNS_TYPE_MX         (15)    /* just kidding */
#define DNS_TYPE_AAAA       (28)
#define DNS_CLASS_IN        (1)

#define DNS_SERV_PORT       (53)
// #define DNS_MAX_RETRIES     (3)

#define DNS_MAX_HOST_NAME_LEN       (64U)
#define DNS_MAX_QUERY_BUFFER_LEN    (sizeof(dns_pkt_t) + 4 + DNS_MAX_HOST_NAME_LEN)
#define DNS_MAX_REPLY_BUFFER_LEN    (1024)
#define DNS_MIN_REPLY_LEN           (ssize_t)(sizeof(dns_pkt_t ) + 7)

int dns_query(const char *name, void *addr_out, int addr_family);

#endif /* DNS_H */
