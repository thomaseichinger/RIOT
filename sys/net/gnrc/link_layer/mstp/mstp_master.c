/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gnrc_mstp
 * @{
 *
 * @file
 * @brief       MS/TP master FSM for the MS/TP MAC layer
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */


#include <stdint.h>

#include "periph/uart.h"
#include "periph/gpio.h"
#include "thread.h"
#include "xtimer.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6/netif.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netapi.h"
#include "net/netopt.h"
#include "net/ieee802154.h"
#include "mstp_internal.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"


#define GNRC_MSTP_MASTER_MSG_QUEUE_SIZE      (8)

msg_t mstp_master_msg;

uint8_t _io_buffer[1500];

static int mstp_send_frame(gnrc_mstp_t *ctx, gnrc_pktsnip_t *pkt);

static void mstp_master_intialize(gnrc_mstp_t *ctx)
{
    return;
}


static int _set(gnrc_mstp_t *ctx, netopt_t opt, void *val, size_t max_len)
{
    if (ctx == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case NETOPT_ADDRESS:
            if (max_len > sizeof(uint8_t)) {
                return -EOVERFLOW;
            }
            ctx->ll_addr = *((uint8_t *) val);
            return sizeof(uint8_t);
        default:
            return -ENOTSUP;
    }
}

static int _get(gnrc_mstp_t *ctx, netopt_t opt, void *val, size_t max_len)
{
    if (ctx == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case NETOPT_ADDRESS:
            if (max_len < sizeof(uint8_t)) {
                return -EOVERFLOW;
            }
            *((uint8_t *)val) = ctx->ll_addr;
            return sizeof(uint8_t);
        case NETOPT_IPV6_IID:
            if (max_len < sizeof(eui64_t)) {
                return -EOVERFLOW;
            }
            *(uint64_t *) val = LOBAC_IID_BASE;
            // ieee802154_get_iid(val, (uint8_t *)&addr, 8);
            return sizeof(eui64_t);
        case NETOPT_MAX_PACKET_SIZE:
            if (max_len < sizeof(int16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = 1500;
            return sizeof(uint16_t);
        case NETOPT_IS_WIRED:
            if (max_len < sizeof(uint8_t)) {
                return -EOVERFLOW;
            }
            *((uint8_t *)val) = 0x01;
            return sizeof(uint8_t);
        case NETOPT_STATE:
            if (max_len < sizeof(netopt_state_t)) {
                return -EOVERFLOW;
            }
            *((netopt_state_t*)val) = NETOPT_STATE_IDLE;
            return sizeof(netopt_state_t);
        case NETOPT_ADDR_LEN:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = 1;
            return sizeof(uint16_t);
        case NETOPT_SRC_LEN:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = 1;
            return sizeof(uint16_t);
        case NETOPT_PROTO:
            if (max_len < sizeof(gnrc_nettype_t)) {
                return -EOVERFLOW;
            }
            *((gnrc_nettype_t *)val) = GNRC_NETTYPE_SIXLOWPAN;
            return sizeof(gnrc_nettype_t);
        default:
            return -ENOTSUP;
    }
}

static void _send_reply(gnrc_mstp_t *ctx) {
    gpio_set(GPIO_PIN(PA, 16));
    gpio_set(GPIO_PIN(PB, 3));

    const uint8_t buffer[] = {MSTP_DATA_PREAMBLE_1, MSTP_DATA_PREAMBLE_2, ctx->frame.type,
                                ctx->frame.dst_addr, ctx->frame.src_addr,
                                (ctx->frame.length>>8), (ctx->frame.length&0xff)};

    uart_write(ctx->uart, &(buffer[0]), 3);
    ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.type, 0xff);

    for (unsigned int i = 3; i < sizeof(buffer); i++) {
        uart_write(ctx->uart, &(buffer[i]), 1);
        ctx->frame.header_crc = mstp_crc_header_update(buffer[i],
                                                       ctx->frame.header_crc);
    }

    uint8_t tmp = ((~(ctx->frame.header_crc))&0xff);
    uart_write(ctx->uart, &tmp, 1);

    /* mstp header */
    // uart_write(ctx->uart, buffer, 1);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, &(buffer[1]), 1);
    // // ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.type, 0xff);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, &(buffer[2]), 1);
    // ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.type, 0xff);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, &(ctx->frame.dst_addr), 1);
    // ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.dst_addr,
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, &(ctx->frame.src_addr), 1);
    // ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.src_addr,
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, (uint8_t*)&(ctx->frame.length), 2);
    // ctx->frame.header_crc = mstp_crc_header_update((ctx->frame.length>>8),
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // // uart_write(ctx->uart, (ctx->frame.length&0xff));
    // ctx->frame.header_crc = mstp_crc_header_update((ctx->frame.length&0xff),
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uint8_t tmp = (~(ctx->frame.header_crc))&0xff;
    // // uint8_t tmp = ctx->frame.header_crc;
    // uart_write(ctx->uart, &tmp, 1);
    xtimer_usleep(220);
    gpio_clear(GPIO_PIN(PB, 3));
    gpio_clear(GPIO_PIN(PA, 16));

    for (int i = 0; i < sizeof(buffer); i++) {
        printf("%02x ", buffer[i]);
    }
    printf("%02x ", tmp);
    puts("RPFM");
}

static void mstp_master_fsm(gnrc_mstp_t *ctx, uint8_t ev)
{
    if (ev == MSTP_EV_RECEIVED_VALID_FRAME) {
        switch (ctx->state) {
            case MSTP_MASTER_IDLE:
                /* we received a valid frame */
                switch (ctx->frame.type) {
                    case MSTP_FRAME_TYPE_TOKEN:
                        ctx->state = MSTP_MASTER_USE_TOKEN;
                        break;
                    case MSTP_FRAME_TYPE_POLL_FOR_MASTER:
                        ctx->frame.dst_addr = ctx->frame.src_addr;
                        ctx->frame.src_addr = ctx->ll_addr;
                        ctx->frame.type = MSTP_FRAME_TYPE_REPLY_POLL_FOR_MASTER;
                        _send_reply(ctx);
                        break;
                    case MSTP_FRAME_TYPE_EXT_DATA_EXP_REPLY:
                        ctx->will_reply = 1;
                        ctx->state = MSTP_MASTER_ANSWER_DATA_REQ;
                        mstp_master_msg.type = MSTP_EV_SUCCESSFULL_RECEPTION;
                        msg_send(&mstp_master_msg, ctx->mac_pid);
                        break;
                }

            case MSTP_MASTER_WAIT_FOR_REPLY:
                switch (ctx->frame.type) {
                    case MSTP_FRAME_TYPE_DATA_NOT_EXP_REPLY:
                    case MSTP_FRAME_TYPE_EXT_DATA_EXP_NO_REPLY:
                        break;
                }
        }
    }
    else if (ev == MSTP_EV_SEND_FRAME) {
        switch (ctx->state) {
            case MSTP_MASTER_ANSWER_DATA_REQ:
            case MSTP_MASTER_USE_TOKEN:
                if (ctx->to_send) {
                    ctx->frame.type = MSTP_FRAME_TYPE_EXT_DATA_EXP_REPLY;
                    mstp_send_frame(ctx, ctx->to_send);
                    ctx->awaiting_reply = 1;
                    ctx->state = MSTP_MASTER_WAIT_FOR_REPLY;
                }
            case MSTP_MASTER_DONE_WITH_TOKEN:
                ctx->to_send = NULL;

            case MSTP_MASTER_PASS_TOKEN:
                ctx->frame.dst_addr = ctx->ns;
                ctx->frame.src_addr = ctx->ll_addr;
                ctx->frame.type = MSTP_FRAME_TYPE_TOKEN;
                _send_reply(ctx);
                break;
            case MSTP_MASTER_POLL_FOR_MASTER:
            case MSTP_MASTER_NO_TOKEN:
                break;
        }
    }
}

// static int mstp_master_handle_ev(gnrc_mstp_t *ctx, uint8_t ev)
// {

//     /* handle event here to send data and replies */
//     if (ev == MSTP_EV_SEND_FRAME) {
//         if (ctx->will_reply) {
//             ctx->frame.type = MSTP_FRAME_TYPE_DATA_NOT_EXP_REPLY;
//             ctx->frame.src_addr = ctx->ll_addr;
//         }
//     }
//     printf("EV: %02x FT: %02x len: %04x\n", ev, ctx->frame.type, ctx->frame.length);
//     if (ctx->frame.type == MSTP_FRAME_TYPE_TOKEN) {
//         ctx->token = 1;
//         ctx->n_poll++;
//         if (ctx->to_send) {
//             ctx->frame.type = MSTP_FRAME_TYPE_EXT_DATA_EXP_REPLY;
//             mstp_send_frame(ctx, ctx->to_send);
//             ctx->awaiting_reply = 1;
//             ctx->to_send = NULL;
//         }
//         if (!(ctx->n_poll<50)) {
//             ctx->n_poll = 0;
//             for (int i = ctx->ll_addr; i<9; i++) {
//                 ctx->frame.type = MSTP_FRAME_TYPE_POLL_FOR_MASTER;
//                 ctx->frame.dst_addr = i;
//                 ctx->frame.src_addr = ctx->ll_addr;
//                 _send_reply(ctx);
//                 xtimer_usleep(2000);
//                 if (ctx->frame.type == MSTP_FRAME_TYPE_REPLY_POLL_FOR_MASTER) {
//                     ctx->ns = ctx->frame.src_addr;
//                     ctx->frame.type = MSTP_FRAME_TYPE_TOKEN;
//                     ctx->frame.dst_addr = ctx->ns;
//                     ctx->frame.src_addr = ctx->ll_addr;
//                     _send_reply(ctx);
//                 }
//             }
//         }
//         else {
//             ctx->frame.dst_addr = ctx->ns;//ctx->frame.src_addr;
//             ctx->frame.src_addr = ctx->ll_addr;
//             ctx->frame.type = MSTP_FRAME_TYPE_TOKEN;
//             // puts("token");
//             _send_reply(ctx);
//         }
//     }
//     else if (ctx->frame.type == MSTP_FRAME_TYPE_DATA_NOT_EXP_REPLY) {
//         if (ctx->awaiting_reply) {
//             /* parse and handle received reply here */
//             puts("rx reply");
//         }
//     }
//     else if (ctx->frame.type == MSTP_FRAME_TYPE_POLL_FOR_MASTER) {
//         puts("PFM");
//         ctx->frame.dst_addr = ctx->frame.src_addr;
//         ctx->frame.src_addr = ctx->ll_addr;
//         ctx->frame.type = MSTP_FRAME_TYPE_REPLY_POLL_FOR_MASTER;
//         _send_reply(ctx);
//         return 0;
//     }
//     else if (ctx->frame.type == MSTP_FRAME_TYPE_DATA_EXP_REPLY) {
//         DEBUG("MSTP: f_t: d e r\n");
//         if (ctx->frame.valid) {
//             ctx->will_reply = 1;
//             mstp_master_msg.type = MSTP_EV_SUCCESSFULL_RECEPTION;
//             msg_send(&mstp_master_msg, ctx->mac_pid);
//         }
//     }
//     else {
//         // puts("unhandled");
//         // printf("t: %02x\n", ctx->frame.type);
//     }
//     return 0;
// }

int mstp_snipify_pkt(gnrc_mstp_t *ctx, gnrc_pktsnip_t *pkt)
{
    return 0;
}

static size_t _make_data_frame_hdr(gnrc_mstp_t *ctx, gnrc_netif_hdr_t *hdr)
{
    /* if AUTOACK is enabled, then we also expect ACKs for this packet */
    // if (!(hdr->flags & GNRC_NETIF_HDR_FLAGS_BROADCAST) &&
    //     !(hdr->flags & GNRC_NETIF_HDR_FLAGS_MULTICAST) &&
    //     (dev->options & AT86RF2XX_OPT_AUTOACK)) {
    //     buf[0] |= IEEE802154_FCF_ACK_REQ;
    // }

    /* fill in destination PAN ID */
    // pos = 3;
    // buf[pos++] = (uint8_t)((dev->pan) & 0xff);
    // buf[pos++] = (uint8_t)((dev->pan) >> 8);
    // ctx->frame.type = MSTP_FRAME_TYPE_DATA_EXP_REPLY;

    /* fill in destination address */
    if (hdr->flags &
        (GNRC_NETIF_HDR_FLAGS_BROADCAST | GNRC_NETIF_HDR_FLAGS_MULTICAST)) {
        ctx->frame.dst_addr = MSTP_BROADCAST_ADDR;
    }
    else if (hdr->dst_l2addr_len == 1) {
        uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);
        ctx->frame.dst_addr = dst_addr[0];
    }
    /* TODO: should not happen and should throw an error */
    else if (hdr->dst_l2addr_len == 8) {
        uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);
        ctx->frame.dst_addr = dst_addr[0];
    }
    else {
        /* unsupported address length */
        DEBUG("mstp: unsupported address length.");
        return 0;
    }

    /* fill in source PAN ID (if applicable */
    // if (dev->options & AT86RF2XX_OPT_USE_SRC_PAN) {
    //     buf[pos++] = (uint8_t)((dev->pan) & 0xff);
    //     buf[pos++] = (uint8_t)((dev->pan) >> 8);
    // } else {
    //     buf[0] |= IEEE802154_FCF_PAN_COMP;
    // }

    /* fill in source address */
    if (ctx->options & IEEE802154_FCF_SRC_ADDR_LONG) {
        /* TODO: remove and throw error if this should ever happen */
        // buf[1] |= IEEE802154_FCF_SRC_ADDR_LONG;
        // memcpy(&(buf[pos]), dev->addr_long, 8);
        // pos += 8;
        ctx->frame.src_addr = ctx->ll_addr;
    }
    else {
        // buf[1] |= IEEE802154_FCF_SRC_ADDR_SHORT;
        // buf[pos++] = dev->addr_short[0];
        // buf[pos++] = dev->addr_short[1];
        ctx->frame.src_addr = ctx->ll_addr;
    }

    // ctx->frame.header_crc = 0x55;

    /* set sequence number */
    // buf[2] = dev->seq_nr++;
    /* return actual header length */
    return 3;
}

static int mstp_send_frame(gnrc_mstp_t *ctx, gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *snip;
    size_t len;

    if (pkt == NULL) {
        return -ENOMSG;
    }
    if (ctx == NULL) {
        gnrc_pktbuf_release(pkt);
        return -ENODEV;
    }

    /* create header */
    len = _make_data_frame_hdr(ctx, (gnrc_netif_hdr_t *)pkt->data);
    if (len == 0) {
        DEBUG("[mstp] error: unable to create header\n");
        gnrc_pktbuf_release(pkt);
        return -ENOMSG;
    }
    /* check if packet (header + payload + FCS) fits into FIFO */
    snip = pkt->next;
    ctx->frame.length = gnrc_pkt_len(snip);
    /* TODO: use define here */
    if (ctx->frame.length > 1500) {
        printf("[mstp] error: packet too large (%u byte) to be send\n",
               ctx->frame.length);
        gnrc_pktbuf_release(pkt);
        return -EOVERFLOW;
    }
    gpio_set(GPIO_PIN(PB, 3));

    const uint8_t buffer[] = {MSTP_DATA_PREAMBLE_1, MSTP_DATA_PREAMBLE_2, ctx->frame.type,
                                ctx->frame.dst_addr, ctx->frame.src_addr,
                                (ctx->frame.length>>8), (ctx->frame.length&0xff)};

    uart_write(ctx->uart, &(buffer[0]), 3);
    ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.type, 0xff);

    for (unsigned int i = 3; i < sizeof(buffer); i++) {
        uart_write(ctx->uart, &(buffer[i]), 1);
        ctx->frame.header_crc = mstp_crc_header_update(buffer[i],
                                                       ctx->frame.header_crc);
    }

    uint8_t tmp = ((~(ctx->frame.header_crc))&0xff);
    uart_write(ctx->uart, &tmp, 1);

    // /* mstp header */
    // uart_write(ctx->uart, MSTP_DATA_PREAMBLE_1);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, MSTP_DATA_PREAMBLE_2);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, ctx->frame.type);
    // ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.type, 0xff);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, ctx->frame.dst_addr);
    // ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.dst_addr,
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, ctx->frame.src_addr);
    // ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.src_addr,
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, (ctx->frame.length>>8));
    // ctx->frame.header_crc = mstp_crc_header_update((ctx->frame.length>>8),
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, (ctx->frame.length&0xff));
    // ctx->frame.header_crc = mstp_crc_header_update((ctx->frame.length&0xff),
    //                                                ctx->frame.header_crc);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);
    // uart_write(ctx->uart, (~(ctx->frame.header_crc))&0xff);
    // // xtimer_usleep(MSTP_T_SEND_WAIT);

    /* load packet data into FIFO */
    uint16_t offset = 0;
    while (snip) {
        mstp_cobs_stuff_data(snip->data, snip->size, _io_buffer+offset);
        offset += snip->size;
        snip = snip->next;
    }
    ctx->frame.data_crc = 0xffffffff;
    for (int i = 0; i<offset; i++) {
        uart_write(ctx->uart, &(_io_buffer[i]), 1);
        ctx->frame.data_crc = mstp_crc_enc_data_update(_io_buffer[i],
                                                       ctx->frame.data_crc);
    }

    tmp = ((ctx->frame.data_crc>>24)&0xff);
    uart_write(ctx->uart, &tmp, 1);
    tmp = ((ctx->frame.data_crc>>16)&0xff);
    uart_write(ctx->uart, &tmp, 1);
    tmp = ((ctx->frame.data_crc>>8)&0xff);
    uart_write(ctx->uart, &tmp, 1);
    tmp = (ctx->frame.data_crc&0xff);
    uart_write(ctx->uart, &tmp, 1);
    /* release packet */
    gnrc_pktbuf_release(pkt);

    // uart_write(ctx->uart, (ctx->frame.data_crc>>8));

    xtimer_usleep(220);
    // uart_write(ctx->uart, (ctx->frame.data_crc&0xff));

    ctx->frame.header_crc = 0xff;
    ctx->frame.data_crc = 0xffff;
    gpio_clear(GPIO_PIN(PB, 3));
    return 0;
}

/**
 * @brief   Startup code and event loop of the MS/TP layer
 *
 * @param[in] args          expects a pointer to the MS/TP context
 *
 * @return                  never returns
 */
static void *_mstp_master_thread(void *args)
{
    gnrc_mstp_t *ctx = (gnrc_mstp_t *)args;
    gnrc_netapi_opt_t *opt;
    int res;
    msg_t msg, reply, msg_queue[GNRC_MSTP_MASTER_MSG_QUEUE_SIZE];

    /* setup the MAC layers message queue */
    msg_init_queue(msg_queue, GNRC_MSTP_MASTER_MSG_QUEUE_SIZE);
    /* save the PID to the device descriptor and register the device */
    ctx->mac_pid = thread_getpid();
    gnrc_netif_add(ctx->mac_pid);
    /* register the event callback with the device driver */
    mstp_master_intialize(ctx);

    /* start the event loop */
    while (1) {
        DEBUG("mstp master: waiting for incoming messages\n");
        msg_receive(&msg);
        /* dispatch NETDEV and NETAPI messages */
        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("mstp master: GNRC_NETAPI_MSG_TYPE_SND received.\n");
                // dev->driver->send_data(dev, (gnrc_pktsnip_t *)msg.content.ptr);
                ctx->to_send = (gnrc_pktsnip_t *)msg.content.ptr;
                // mstp_send_frame(ctx, (gnrc_pktsnip_t *)msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_SET:
                /* TODO: filter out MAC layer options -> for now forward
                         everything to the device driver */
                DEBUG("mstp master: GNRC_NETAPI_MSG_TYPE_SET received\n");
                /* read incoming options */
                opt = (gnrc_netapi_opt_t *)msg.content.ptr;
                /* set option for device driver */
                res = _set(ctx, opt->opt, opt->data, opt->data_len);
                DEBUG("mstp master: response of netdev->set: %i\n", res);
                /* send reply to calling thread */
                reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
                /* TODO: filter out MAC layer options -> for now forward
                         everything to the device driver */
                DEBUG("mstp master: GNRC_NETAPI_MSG_TYPE_GET received\n");
                /* read incoming options */
                opt = (gnrc_netapi_opt_t *)msg.content.ptr;
                /* get option from device driver */
                res = _get(ctx, opt->opt, opt->data, opt->data_len);
                DEBUG("mstp master: response of netdev->get: %i\n", res);
                /* send reply to calling thread */
                reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
                uint32_t tmp = res;
                reply.content.value = tmp;
                msg_reply(&msg, &reply);
                break;
            case MSTP_EV_RECEIVED_VALID_FRAME:
                DEBUG("mstp master: MSTP_EV_RECEIVED_VALID_FRAME received\n");
                mstp_master_fsm(ctx, MSTP_EV_RECEIVED_VALID_FRAME);
                break;
            case MSTP_EV_SUCCESSFULL_RECEPTION:
                DEBUG("mstp master: MSTP_EV_SUCCESSFULL_RECEPTION received\n");
                gnrc_pktsnip_t *pkt = gnrc_pktbuf_add(NULL, NULL, ctx->frame.length + 5, GNRC_NETTYPE_UNDEF);
                mstp_snipify_pkt((gnrc_mstp_t *)ctx, pkt);
                /* send the packet to everyone interested in it's type */
                if (!gnrc_netapi_dispatch_receive(pkt->type, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
                    DEBUG("mstp master: unable to forward packet of type %i\n", pkt->type);
                    gnrc_pktbuf_release(pkt);
                }
            case MSTP_EV_T_FRAME_ABORT:
                puts("mstp master: frame abort timer fired, take care of it!");
                ctx->state = MSTP_STATE_IDLE;
                memset(&(ctx->frame), 0, sizeof(mstp_frame_t));
                break;
            default:
                printf("mstp master: Unknown command %" PRIu16 " from %" PRIkernel_pid "\n", msg.type, msg.sender_pid);
                break;
        }
    }
    /* never reached */
    return NULL;
}

int gnrc_mstp_init(gnrc_mstp_t *ctx, const gnrc_mstp_params_t *p)
{
    ctx->uart = p->uart;

    uart_init(ctx->uart, p->baudrate, mstp_receive_frame, (void *)ctx);
    int err = gpio_init(GPIO_PIN(PB, 3), GPIO_OUT);
    if (err < 0) {
        puts("error!!!");
        xtimer_usleep(1000000);
    }
    err = gpio_init(GPIO_PIN(PA, 16), GPIO_OUT);
    if (err < 0) {
        puts("error!!!");
        xtimer_usleep(1000000);
    }
    ctx->msg_fa.type = MSTP_EV_T_FRAME_ABORT;
    ctx->ll_addr = 0x03;

    return 0;
}

kernel_pid_t gnrc_mstp_start(char *stack, int stacksize, char priority,
                            const char *name, gnrc_mstp_t *ctx)
{
    kernel_pid_t res;

    /* check if given mstp context is defined and the frame to use is set */
    if (ctx == NULL) {
        return -ENODEV;
    }
    /* create new mstp master thread */
    res = thread_create(stack, stacksize, priority, THREAD_CREATE_STACKTEST,
                        _mstp_master_thread, (void *)ctx, name);
    if (res <= 0) {
        return -EINVAL;
    }
    return res;
}
