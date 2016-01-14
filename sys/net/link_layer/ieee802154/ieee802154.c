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
 * @author Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <assert.h>
#include <errno.h>

#include "net/ieee802154.h"

size_t ieee802154_set_frame_hdr(uint8_t *buf, const ieee802154_addr_t *src,
                                const ieee802154_addr_t *dst,
                                network_uint16_t flags, uint8_t seq)
{
    uint8_t addr_flags = flags.u8[1];
    uint8_t type = (flags.u8[0] & IEEE802154_FCF_TYPE_MASK);
    int pos = 3;    /* 0-1: FCS, 2: seq */

    if (((addr_flags & IEEE802154_FCF_SRC_ADDR_MASK) != IEEE802154_FCF_SRC_ADDR_VOID) &&
        (src == NULL)) {    /* dst == NULL implies broadcast so we don't need to check */
        return 0;
    }

    if (((addr_flags & IEEE802154_FCF_DST_ADDR_MASK) == IEEE802154_FCF_DST_ADDR_RESV) ||
        ((addr_flags & IEEE802154_FCF_SRC_ADDR_MASK) == IEEE802154_FCF_SRC_ADDR_RESV)) {
        return 0;
    }

    /* Frame type is not beacon or ACK, but both address modes are zero */
    if ((type != IEEE802154_FCF_TYPE_BEACON) && (type != IEEE802154_FCF_TYPE_ACK) &&
        ((addr_flags & IEEE802154_FCF_DST_ADDR_MASK) == IEEE802154_FCF_DST_ADDR_VOID) &&
        ((addr_flags & IEEE802154_FCF_SRC_ADDR_MASK) == IEEE802154_FCF_SRC_ADDR_VOID)) {
        return 0;
    }

    buf[0] = flags.u8[0];
    buf[1] = addr_flags;
    /* set sequence number */
    buf[2] = seq;

    /* fill in destination address */
    if ((dst == NULL) &&    /* multicast destiation */
        ((addr_flags & IEEE802154_FCF_DST_ADDR_MASK) != IEEE802154_FCF_DST_ADDR_VOID)) {
        /* no AUTOACK for broadcast */
        buf[0] &= ~IEEE802154_FCF_ACK_REQ;
        buf[1] &= ~IEEE802154_FCF_DST_ADDR_MASK;
        buf[1] |= IEEE802154_FCF_DST_ADDR_SHORT;
        buf[pos++] = src->pan.u8[1];
        buf[pos++] = src->pan.u8[0];
        buf[pos++] = 0xff;
        buf[pos++] = 0xff;
    }
    else if ((addr_flags & IEEE802154_FCF_DST_ADDR_MASK) == IEEE802154_FCF_DST_ADDR_SHORT) {
        /* fill in destination address little-endian */
        buf[pos++] = dst->pan.u8[1];
        buf[pos++] = dst->pan.u8[0];
        buf[pos++] = dst->addr.s_addr.u8[1];
        buf[pos++] = dst->addr.s_addr.u8[0];
    }
    else if ((addr_flags & IEEE802154_FCF_DST_ADDR_MASK) == IEEE802154_FCF_DST_ADDR_LONG) {
        buf[pos++] = dst->pan.u8[1];
        buf[pos++] = dst->pan.u8[0];
        for (int i = 7; i >= 0; i--) {
            buf[pos++] = dst->addr.l_addr.u8[i];
        }
    }
    else if ((addr_flags & IEEE802154_FCF_DST_ADDR_MASK) != IEEE802154_FCF_DST_ADDR_VOID) {
        /* unsupported address mode */
        return 0;
    }

    /* fill in source PAN ID (if applicable) */
    if (!(flags.u8[0] & IEEE802154_FCF_PAN_COMP) &&
        ((addr_flags & IEEE802154_FCF_SRC_ADDR_MASK) != IEEE802154_FCF_SRC_ADDR_VOID)) {
        /* (little endian) */
        buf[pos++] = src->pan.u8[1];
        buf[pos++] = src->pan.u8[0];
    }

    /* fill in source address */
    if ((addr_flags & IEEE802154_FCF_SRC_ADDR_MASK) == IEEE802154_FCF_SRC_ADDR_SHORT) {
        /* fill in destination address little-endian */
        buf[pos++] = src->addr.s_addr.u8[1];
        buf[pos++] = src->addr.s_addr.u8[0];
    }
    else if ((addr_flags & IEEE802154_FCF_SRC_ADDR_MASK) == IEEE802154_FCF_SRC_ADDR_LONG) {
        for (int i = 7; i >= 0; i--) {
            buf[pos++] = src->addr.l_addr.u8[i];
        }
    }
    else if ((addr_flags & IEEE802154_FCF_SRC_ADDR_MASK) != IEEE802154_FCF_SRC_ADDR_VOID) {
        /* unsupported address mode */
        return 0;
    }

    /* return actual header length */
    return pos;
}

size_t ieee802154_get_frame_hdr_len(const uint8_t *mhr)
{
    /* TODO: include security header implications */
    uint8_t tmp;
    size_t len = 3; /* 2 byte FCF, 1 byte sequence number */

    /* figure out address sizes */
    tmp = (mhr[1] & IEEE802154_FCF_DST_ADDR_MASK);
    if (tmp == IEEE802154_FCF_DST_ADDR_SHORT) {
        len += 4;   /* 2 byte dst PAN + 2 byte dst short address */
    }
    else if (tmp == IEEE802154_FCF_DST_ADDR_LONG) {
        len += 10;  /* 2 byte dst PAN + 2 byte dst long address */
    }
    else if (tmp != IEEE802154_FCF_DST_ADDR_VOID) {
        return 0;
    }
    else if (mhr[0] & IEEE802154_FCF_PAN_COMP) {
        /* PAN compression, but no destination address => illegal state */
        return 0;
    }
    tmp = (mhr[1] & IEEE802154_FCF_SRC_ADDR_MASK);
    if (tmp == IEEE802154_FCF_SRC_ADDR_VOID) {
        return len;
    }
    else {
        if (!(mhr[0] & IEEE802154_FCF_PAN_COMP)) {
            len += 2;
        }
        if (tmp == IEEE802154_FCF_SRC_ADDR_SHORT) {
            return len + 2;
        }
        else if (tmp == IEEE802154_FCF_SRC_ADDR_LONG) {
            return len + 8;
        }
    }
    return 0;
}

int ieee802154_get_addr(const uint8_t *mhr, ieee802154_addr_t *src,
                        ieee802154_addr_t *dst)
{
    int res, offset = 3;    /* FCF: 0-1, Seq: 2 */
    uint8_t tmp;

    assert((src != NULL) && (dst != NULL));
    tmp = mhr[1] & IEEE802154_FCF_DST_ADDR_MASK;
    if (tmp == IEEE802154_FCF_DST_ADDR_SHORT) {
        /* read dst PAN and address in little endian */
        dst->pan.u8[1] = mhr[offset++];
        dst->pan.u8[0] = mhr[offset++];
        dst->addr.s_addr.u8[1] = mhr[offset++];
        dst->addr.s_addr.u8[0] = mhr[offset++];
    }
    else if (tmp == IEEE802154_FCF_DST_ADDR_LONG) {
        dst->pan.u8[1] = mhr[offset++];
        dst->pan.u8[0] = mhr[offset++];
        for (int i = 7; i >= 0; i--) {
            dst->addr.l_addr.u8[i] = mhr[offset++];
        }

    }
    else if (tmp != IEEE802154_FCF_DST_ADDR_VOID) {
        return -EINVAL;
    }
    else if (mhr[0] & IEEE802154_FCF_PAN_COMP) {
        /* PAN compression, but no destination address => illegal state */
        return -EINVAL;
    }
    res = (int)tmp;

    tmp = mhr[1] & IEEE802154_FCF_SRC_ADDR_MASK;
    if (tmp != IEEE802154_FCF_SRC_ADDR_VOID) {
        if (mhr[0] & IEEE802154_FCF_PAN_COMP) {
            src->pan.u16 = dst->pan.u16;
        }
        else {
            src->pan.u8[1] = mhr[offset++];
            src->pan.u8[0] = mhr[offset++];
        }
    }
    if (tmp == IEEE802154_FCF_SRC_ADDR_SHORT) {
        /* read src PAN and address in little endian */
        src->addr.s_addr.u8[1] = mhr[offset++];
        src->addr.s_addr.u8[0] = mhr[offset++];
    }
    else if (tmp == IEEE802154_FCF_SRC_ADDR_LONG) {
        /* read src PAN and address in little endian */
        for (int i = 7; i >= 0; i--) {
            src->addr.l_addr.u8[i] = mhr[offset++];
        }

    }
    else if (tmp != IEEE802154_FCF_SRC_ADDR_VOID) {
        return -EINVAL;
    }
    res |= (int)tmp;
    return res;
}

/** @} */
