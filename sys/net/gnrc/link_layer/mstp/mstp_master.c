#include <stdint.h>

#include "periph/uart.h"
#include "thread.h"
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
        default:
            return -ENOTSUP;
    }
}

static int mstp_master_handle_ev(gnrc_mstp_t *ctx, uint8_t ev)
{
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
    else if (hdr->dst_l2addr_len == 2) {
        uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);
        // buf[1] |= IEEE802154_FCF_DST_ADDR_SHORT;
        // buf[pos++] = dst_addr[1];
        // buf[pos++] = dst_addr[0];
        ctx->frame.dst_addr = dst_addr[0];
    }
    else if (hdr->dst_l2addr_len == 8) {
        // buf[1] |= IEEE802154_FCF_DST_ADDR_LONG;
        uint8_t *dst_addr = gnrc_netif_hdr_get_dst_addr(hdr);
        // for (int i = 7;  i >= 0; i--) {
        //     buf[pos++] = dst_addr[i];
        // }
        ctx->frame.dst_addr = dst_addr[0];
    }
    else {
        /* unsupported address length */
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
        ctx->frame.src_addr = ctx->addr;
    }
    else {
        // buf[1] |= IEEE802154_FCF_SRC_ADDR_SHORT;
        // buf[pos++] = dev->addr_short[0];
        // buf[pos++] = dev->addr_short[1];
        ctx->frame.src_addr = ctx->addr;
    }

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
    ctx->frame.length = gnrc_pkt_len(snip) + len + 2;
    if (ctx->frame.length > 1500) {
        printf("[mstp] error: packet too large (%u byte) to be send\n",
               ctx->frame.length);
        gnrc_pktbuf_release(pkt);
        return -EOVERFLOW;
    }

    /* mstp header */
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_1);
    uart_write_blocking(ctx->uart, MSTP_DATA_PREAMBLE_2);
    uart_write_blocking(ctx->uart, ctx->frame.type);
    uart_write_blocking(ctx->uart, ctx->frame.dst_addr);
    uart_write_blocking(ctx->uart, ctx->frame.src_addr);
    uart_write_blocking(ctx->uart, (ctx->frame.length>>8));
    uart_write_blocking(ctx->uart, (ctx->frame.length&0xff));
    uart_write_blocking(ctx->uart, ctx->frame.header_crc);

    /* load packet data into FIFO */
    while (snip) {
        for (uint16_t i = 0; i < snip->size; i++) {
            uart_write_blocking(ctx->uart, ((char *)snip->data)[i]);
        }
        snip = snip->next;
    }

    /* release packet */
    gnrc_pktbuf_release(pkt);

    uart_write_blocking(ctx->uart, (ctx->frame.data_crc>>8));
    uart_write_blocking(ctx->uart, (ctx->frame.data_crc&0xff));
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
                DEBUG("mstp master: GNRC_NETAPI_MSG_TYPE_SND received (NOT IMPLEMENTED)\n");
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

            default:
                DEBUG("mstp master: Unknown command %" PRIu16 "\n", msg.type);
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
