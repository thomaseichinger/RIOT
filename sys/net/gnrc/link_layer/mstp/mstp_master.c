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
        default:
            return -ENOTSUP;
    }
}

static void _send_reply_pfm(gnrc_mstp_t *ctx) {
    gpio_set(GPIO(PA, 22));
    gpio_set(GPIO(PB, 3));
    /* mstp header */
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_1);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_2);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.type);
    ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.type, 0xff);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.dst_addr);
    ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.dst_addr,
                                                   ctx->frame.header_crc);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.src_addr);
    ctx->frame.header_crc = mstp_crc_header_update(ctx->frame.src_addr,
                                                   ctx->frame.header_crc);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, (ctx->frame.length>>8));
    ctx->frame.header_crc = mstp_crc_header_update((ctx->frame.length>>8),
                                                   ctx->frame.header_crc);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, (ctx->frame.length&0xff));
    ctx->frame.header_crc = mstp_crc_header_update((ctx->frame.length&0xff),
                                                   ctx->frame.header_crc);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.header_crc);
    xtimer_usleep(120);
    gpio_clear(GPIO(PB, 3));
    gpio_clear(GPIO(PA, 22));
}

static int mstp_master_handle_ev(gnrc_mstp_t *ctx, uint8_t ev)
{
    gpio_set(GPIO(PA, 22));
    if (ev != MSTP_EV_RECEIVED_VALID_FRAME) {
        puts("Sorry, event no have!");
        return -1;
    }
    // printf("EV: %02x FT: %02x\n", ev, ctx->frame.type);

    if (ctx->frame.type == MSTP_FRAME_TYPE_POLL_FOR_MASTER) {
        puts("PFM");
        ctx->frame.dst_addr = ctx->frame.src_addr;
        ctx->frame.src_addr = ctx->ll_addr;
        ctx->frame.type = MSTP_FRAME_TYPE_REPLY_POLL_FOR_MASTER;
        _send_reply_pfm(ctx);
        return 0;
    }
    else if (ctx->frame.type == MSTP_FRAME_TYPE_DATA_EXP_REPLY) {
        DEBUG("MSTP: f_t: d e r\n");
        if (ctx->frame.valid) {
            mstp_master_msg.type = MSTP_EV_SUCCESSFULL_RECEPTION;
            msg_send(&mstp_master_msg, ctx->mac_pid);
        }
    }
    else {
        puts("unhandled");
        // printf("t: %02x\n", ctx->frame.type);
    }
    gpio_clear(GPIO(PA, 22));
    return 0;
}

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
    ctx->frame.type = MSTP_FRAME_TYPE_DATA_EXP_REPLY;

    /* fill in destination address */
    if (hdr->flags &
        (GNRC_NETIF_HDR_FLAGS_BROADCAST | GNRC_NETIF_HDR_FLAGS_MULTICAST)) {
        ctx->frame.dst_addr = MSTP_BROADCAST_ADDR;
    }
    else if (hdr->dst_l2addr_len == 1) {
        uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);
        ctx->frame.dst_addr = dst_addr[0];
    }
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

    ctx->frame.header_crc = 0x55;

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

    /* create 802.15.4 header */
    len = _make_data_frame_hdr(ctx, (gnrc_netif_hdr_t *)pkt->data);
    if (len == 0) {
        DEBUG("[mstp] error: unable to create 802.15.4 header\n");
        gnrc_pktbuf_release(pkt);
        return -ENOMSG;
    }
    /* check if packet (header + payload + FCS) fits into FIFO */
    snip = pkt->next;
    ctx->frame.length = gnrc_pkt_len(snip);
    if (ctx->frame.length > 1500) {
        printf("[mstp] error: packet too large (%u byte) to be send\n",
               ctx->frame.length);
        gnrc_pktbuf_release(pkt);
        return -EOVERFLOW;
    }
    gpio_set(GPIO(PB, 3));
    /* mstp header */
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_1);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_2);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.type);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.dst_addr);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.src_addr);
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, (ctx->frame.length>>8));
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, (ctx->frame.length&0xff));
    // xtimer_usleep(MSTP_T_SEND_WAIT);
    uart_write_blocking(ctx->uart, ctx->frame.header_crc);
    // xtimer_usleep(MSTP_T_SEND_WAIT);

    /* load packet data into FIFO */
    while (snip) {
        for (uint16_t i = 0; i < snip->size; i++) {
            ctx->frame.data_crc = mstp_crc_data_update(((uint8_t *)snip->data)[i],
                                                      ctx->frame.data_crc);
            uart_write_blocking(ctx->uart, ((char *)snip->data)[i]);
            // xtimer_usleep(MSTP_T_SEND_WAIT*4);
        }
        snip = snip->next;
    }
    uart_write_blocking(ctx->uart, 0xf0);
    uart_write_blocking(ctx->uart, 0xb8);
    /* release packet */
    gnrc_pktbuf_release(pkt);

    // uart_write_blocking(ctx->uart, (ctx->frame.data_crc>>8));

    xtimer_usleep(120);
    // uart_write_blocking(ctx->uart, (ctx->frame.data_crc&0xff));

    ctx->frame.header_crc = 0xff;
    ctx->frame.data_crc = 0xffff;
    gpio_clear(GPIO(PB, 3));
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
                mstp_send_frame(ctx, (gnrc_pktsnip_t *)msg.content.ptr);
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
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case MSTP_EV_RECEIVED_VALID_FRAME:
                DEBUG("mstp master: MSTP_EV_RECEIVED_VALID_FRAME received\n");
                mstp_master_handle_ev(ctx, MSTP_EV_RECEIVED_VALID_FRAME);
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

    uart_init(ctx->uart, p->baudrate, mstp_receive_frame, NULL, (void *)ctx);
    gpio_init(GPIO(PB, 3), GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_init(GPIO(PA, 22), GPIO_DIR_OUT, GPIO_NOPULL);
    ctx->msg_fa.type = MSTP_EV_T_FRAME_ABORT;

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
    res = thread_create(stack, stacksize, priority, CREATE_STACKTEST,
                        _mstp_master_thread, (void *)ctx, name);
    if (res <= 0) {
        return -EINVAL;
    }
    return res;
}
