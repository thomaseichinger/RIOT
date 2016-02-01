/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <stddef.h>

#include "od.h"
#include "net/gnrc.h"
#include "net/ieee802154.h"

#include "net/gnrc/netdev2/ieee802154.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define _TAIL_LEN       (4)
#define _LQI_TAIL_POS   (2)
#define _RSSI_TAIL_POS  (3)

static gnrc_pktsnip_t *_recv(gnrc_netdev2_t *gnrc_netdev2);
static int _send(gnrc_netdev2_t *gnrc_netdev2, gnrc_pktsnip_t *pkt);

int gnrc_netdev2_ieee802154_init(gnrc_netdev2_t *gnrc_netdev2,
                                 netdev2_ieee802154_t *dev)
{
    gnrc_netdev2->send = _send;
    gnrc_netdev2->recv = _recv;
    gnrc_netdev2->dev = (netdev2_t *)dev;

    return 0;
}

static gnrc_pktsnip_t *_make_netif_hdr(uint8_t *mhr)
{
    gnrc_pktsnip_t *snip;
    uint8_t src[IEEE802154_LONG_ADDRESS_LEN], dst[IEEE802154_LONG_ADDRESS_LEN];
    int src_len, dst_len;
    le_uint16_t src_pan, dst_pan;   /* TODO: hand-up PAN IDs? */

    dst_len = ieee802154_get_dst(mhr, dst, &dst_pan);
    src_len = ieee802154_get_src(mhr, src, &src_pan);
    if ((dst_len < 0) || (src_len < 0)) {
        DEBUG("_make_netif_hdr: unable to get addresses\n");
        return NULL;
    }
    /* allocate space for header */
    snip = gnrc_netif_hdr_build(src, (size_t)src_len, dst, (size_t)dst_len);
    if (snip == NULL) {
        DEBUG("_make_netif_hdr: no space left in packet buffer\n");
        return NULL;
    }
    /* set broadcast flag for broadcast destination */
    if ((dst_len == 2) && (dst[0] == 0xff) && (dst[1] == 0xff)) {
        gnrc_netif_hdr_t *hdr = snip->data;
        hdr->flags |= GNRC_NETIF_HDR_FLAGS_BROADCAST;
    }
    return snip;
}

static gnrc_pktsnip_t *_recv(gnrc_netdev2_t *gnrc_netdev2)
{
    netdev2_ieee802154_t *dev = (netdev2_ieee802154_t *)gnrc_netdev2->dev;
    gnrc_pktsnip_t *pkt = NULL;
    int bytes_expected = dev->driver->recv((netdev2_t *)dev, NULL, 0);

    if (bytes_expected > 0) {
        int nread;

        pkt = gnrc_pktbuf_add(NULL, NULL, bytes_expected, GNRC_NETTYPE_UNDEF);
        if (pkt == NULL) {
            DEBUG("_recv_ieee802154: cannot allocate pktsnip.\n");
            return NULL;
        }
        nread = dev->driver->recv((netdev2_t *)dev, pkt->data, bytes_expected) - _TAIL_LEN;
        if (nread <= 0) {
            gnrc_pktbuf_release(pkt);
            return NULL;
        }
        if (!(dev->flags & NETDEV2_IEEE802154_RAW)) {
            gnrc_pktsnip_t *ieee802154_hdr, *netif_hdr;
            gnrc_netif_hdr_t *hdr;
            uint8_t *data;
#if ENABLE_DEBUG
            char src_str[GNRC_NETIF_HDR_L2ADDR_MAX_LEN];
#endif
            size_t mhr_len = ieee802154_get_frame_hdr_len(pkt->data);

            if (mhr_len == 0) {
                DEBUG("_recv_ieee802154: illegally formatted frame received\n");
                gnrc_pktbuf_release(pkt);
                return NULL;
            }
            nread -= mhr_len;
            /* mark IEEE 802.15.4 header */
            ieee802154_hdr = gnrc_pktbuf_mark(pkt, mhr_len, GNRC_NETTYPE_UNDEF);
            if (ieee802154_hdr == NULL) {
                DEBUG("_recv_ieee802154: no space left in packet buffer\n");
                gnrc_pktbuf_release(pkt);
                return NULL;
            }
            netif_hdr = _make_netif_hdr(ieee802154_hdr->data);
            if (netif_hdr == NULL) {
                DEBUG("_recv_ieee802154: no space left in packet buffer\n");
                gnrc_pktbuf_release(pkt);
                return NULL;
            }
            hdr = netif_hdr->data;
            data = pkt->data;
            hdr->lqi = data[nread + _LQI_TAIL_POS];
            hdr->rssi = data[nread + _RSSI_TAIL_POS];
            hdr->if_pid = thread_getpid();
            pkt->type = dev->proto;
#if ENABLE_DEBUG
            DEBUG("_recv_ieee802154: received packet from %s of length %u\n",
                  gnrc_netif_addr_to_str(src_str, sizeof(src_str),
                                         gnrc_netif_hdr_get_src_addr(hdr),
                                         hdr->src_l2addr_len),
                  nread);
#if defined(MODULE_OD)
            od_hex_dump(pkt->data, nread, OD_WIDTH_DEFAULT);
#endif
#endif
            gnrc_pktbuf_remove_snip(pkt, ieee802154_hdr);
            LL_APPEND(pkt, netif_hdr);
        }

        DEBUG("_recv_ieee802154: reallocating.\n");
        gnrc_pktbuf_realloc_data(pkt, nread);
    }

    return pkt;
}

static int _send(gnrc_netdev2_t *gnrc_netdev2, gnrc_pktsnip_t *pkt)
{
    netdev2_ieee802154_t *dev = (netdev2_ieee802154_t *)gnrc_netdev2->dev;
    gnrc_netif_hdr_t *netif_hdr;
    struct iovec *vector;
    uint8_t *src, *dst = NULL;
    int res = 0;
    size_t n, src_len;
    uint8_t mhr[IEEE802154_MAX_HDR_LEN];
    uint8_t flags = (uint8_t)(dev->flags & NETDEV2_IEEE802154_SEND_MASK);
    le_uint16_t dev_pan = byteorder_btols(byteorder_htons(dev->pan));

    flags |= IEEE802154_FCF_TYPE_DATA;
    if (pkt == NULL) {
        DEBUG("_send_ieee802154: pkt was NULL\n");
        return -EINVAL;
    }
    if (pkt->type != GNRC_NETTYPE_NETIF) {
        DEBUG("_send_ieee802154: first header is not generic netif header\n");
        return -EBADMSG;
    }
    netif_hdr = pkt->data;
    /* prepare destination address */
    if (netif_hdr->flags & /* If any of these flags is set so this is correct */
        (GNRC_NETIF_HDR_FLAGS_BROADCAST | GNRC_NETIF_HDR_FLAGS_MULTICAST)) {
        flags |= IEEE802154_BCAST;
    }
    else {
        dst = gnrc_netif_hdr_get_dst_addr(netif_hdr);
    }
    src_len = netif_hdr->src_l2addr_len;
    if (src_len > 0) {
        src = gnrc_netif_hdr_get_src_addr(netif_hdr);
    }
    else if (dev->flags & NETDEV2_IEEE802154_SRC_MODE_LONG) {
        src_len = IEEE802154_LONG_ADDRESS_LEN;
        src = dev->long_addr;
    }
    else {
        src_len = IEEE802154_SHORT_ADDRESS_LEN;
        src = dev->short_addr;
    }
    /* fill MAC header, seq should be set by device */
    if ((res = ieee802154_set_frame_hdr(mhr, src, src_len,
                                        dst, netif_hdr->dst_l2addr_len,
                                        dev_pan, dev_pan, flags,
                                        dev->seq++)) == 0) {
        DEBUG("_send_ieee802154: Error preperaring frame\n");
        return -EINVAL;
    }
    /* prepare packet for sending */
    pkt = gnrc_pktbuf_get_iovec(pkt, &n);
    vector = (struct iovec *)pkt->data;
    vector[0].iov_base = mhr;
    vector[0].iov_len = (size_t)res;
    dev->driver->send((netdev2_t *)dev, vector, n);
    /* release old data */
    gnrc_pktbuf_release(pkt);
    return 0;
}

/** @} */
