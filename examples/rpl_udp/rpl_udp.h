/*
 * Copyright (C) 2014  Oliver Hahm <oliver.hahm@inria.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef RPL_UDP_H
#define RPL_UDP_H

#ifdef __cplusplus
extern "C" {
#endif

#define APP_VERSION "1.2"

#define RADIO_CHANNEL   (12)

#define MONITOR_STACK_SIZE  (KERNEL_CONF_STACKSIZE_MAIN)
#define RCV_BUFFER_SIZE     (32)


/****** CONFIGURATION APP RPL_UDP ******/
#define PASSERELLE                  1
#define NODE                        0

#define ADDRESS_IPV6_BOARD          1
#define ADDRESS_ROOM                1
#define ADDRESS_PASSERELLE          1
/****** CONFIGURATION APP RPL_UDP ******/

/* RPL shell command handlers */
/**
 * @brief   Shell command to initializes RPL and UDP
 *
 * @details Usage: init <r|n>
 *                 `init r` will initialize the node as a RPL root node,
 *                 `init n` as a RPL node.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void rpl_udp_init(void);

/**
 * @brief   Shell command to set node's ID
 *
 * @details Usage: set <ID>
 *          Set the node address
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void rpl_udp_set_id(int i);


/**
 * @brief Command handler to start a UDP server
 *
 * @details No parameters required
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void udp_server(void);

/**
 * @brief Sends a UDP datagram
 *
 * @details Usage: send <ID> <TEXT>
 *          Sends TEXT to the node with IP address:
 *          fe80::ff:fe00:<ID>
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void udp_send(int add,char *c);


/** @brief The nodes radio address */
extern radio_address_t id;

/** @brief Char array for IP address printing */
//extern char addr_str[IPV6_MAX_ADDR_STR_LEN];

#ifdef __cplusplus
}
#endif

#endif /* RPL_UDP_H */
