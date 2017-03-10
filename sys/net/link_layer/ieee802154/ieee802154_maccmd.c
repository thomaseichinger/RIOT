/*
 * Copyright (C) 2017 Thomas Eichinger
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author Thomas Eichinger <thomas@riot-os.org>
 */

#include <errno.h>

#include "net/ieee802154.h"
#include "net/ieee802154_maccmd.h"

int ieee802154_mac_beacon_req(uint8_t *out_buf, size_t *out_len, uint8_t seq)
{
    if (*out_len < 7) {
        return -EINVAL;
    }

    uint8_t *pkt = out_buf;
    unsigned int cntr;
    uint16_t src_addr = 0;
    uint16_t dst_addr = 0xffff;

    /* specify FCF */
    uint8_t flags = IEEE802154_FCF_TYPE_MACCMD 
                  | IEEE802154_FCF_SRC_ADDR_VOID
                  | IEEE802154_FCF_DST_ADDR_SHORT;

    cntr = ieee802154_set_frame_hdr(out_buf, NULL, 0, ieee802154_addr_bcast, 
                                    2, byteorder_btols(byteorder_htons(src_addr)), 
                                    byteorder_btols(byteorder_htons(dst_addr)), flags, seq);

    /* set MAC cmd to beacon request */
    pkt[cntr] = IEEE802154_MACCMD_BEACON_REQ;

    return cntr;
}

/** @} */
