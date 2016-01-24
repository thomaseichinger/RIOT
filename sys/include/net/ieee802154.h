/*
 * Copyright (C) 2015-16 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ieee802154 IEEE802.15.4
 * @ingroup     net
 * @brief       IEEE802.15.4 header definitions and utility functions
 * @{
 *
 * @file
 * @brief       IEEE 802.14.4 header definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef IEEE802154_H_
#define IEEE802154_H_

#include <stdint.h>
#include <stdlib.h>

#include "byteorder.h"
#include "net/eui64.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief IEEE 802.15.4 address lengths
 * @{
 */
#define IEEE802154_SHORT_ADDRESS_LEN        (2U)    /**< short (16-bit) address */
#define IEEE802154_LONG_ADDRESS_LEN         (8U)    /**< long address (EUI-64) */
/**
 * @}
 */

/**
 * @brief IEEE802.15.4 FCF field definitions
 * @{
 */
#define IEEE802154_MAX_HDR_LEN              (23U)
#define IEEE802154_FCF_LEN                  (2U)
#define IEEE802154_FCS_LEN                  (2U)

#define IEEE802154_FCF_TYPE_MASK            (0x07)
#define IEEE802154_FCF_TYPE_BEACON          (0x00)
#define IEEE802154_FCF_TYPE_DATA            (0x01)
#define IEEE802154_FCF_TYPE_ACK             (0x02)
#define IEEE802154_FCF_TYPE_MACCMD          (0x03)

#define IEEE802154_FCF_SECURITY_EN          (0x08)
#define IEEE802154_FCF_FRAME_PEND           (0x10)
#define IEEE802154_FCF_ACK_REQ              (0x20)
#define IEEE802154_FCF_PAN_COMP             (0x40)

#define IEEE802154_FCF_DST_ADDR_MASK        (0x0c)
#define IEEE802154_FCF_DST_ADDR_VOID        (0x00)
#define IEEE802154_FCF_DST_ADDR_RESV        (0x04)
#define IEEE802154_FCF_DST_ADDR_SHORT       (0x08)
#define IEEE802154_FCF_DST_ADDR_LONG        (0x0c)

#define IEEE802154_FCF_VERS_MASK            (0x30)
#define IEEE802154_FCF_VERS_V0              (0x00)
#define IEEE802154_FCF_VERS_V1              (0x10)

#define IEEE802154_FCF_SRC_ADDR_MASK        (0xc0)
#define IEEE802154_FCF_SRC_ADDR_VOID        (0x00)
#define IEEE802154_FCF_SRC_ADDR_RESV        (0x40)
#define IEEE802154_FCF_SRC_ADDR_SHORT       (0x80)
#define IEEE802154_FCF_SRC_ADDR_LONG        (0xc0)
/** @} */

/**
 * @brief   Unified address representation for IEEE 802.15.4 frames.
 */
typedef struct {
    network_uint16_t pan;           /**< PAN ID */
    /* address part of address */
    union {
        network_uint64_t l_addr;    /**< long address format */
        network_uint16_t s_addr;    /**< short address format */
    } addr;
} ieee802154_addr_t;

/**
 * @brief   Initializes an IEEE 802.15.4 MAC frame header in @p buf.
 *
 * @pre Resulting header must fit in memory allocated at @p buf.
 *
 * @see IEEE Std 802.15.4-2011, 5.2.1 General MAC frame format.
 *
 * If @p dst is NULL the IEEE802154_FCF_ACK_REQ will be unset to prevent
 * flooding the network.
 *
 * @param[out] buf  Target memory for frame header.
 * @param[in] src   Source address for frame. May be NULL if
 *                  IEEE802154_FCF_SRC_ADDR_VOID is set in @p flags.
 * @param[in] dst   Destination address for frame. May be NULL for broadcast
 *                  or if IEEE802154_FCF_DST_ADDR_VOID is set in @p flags.
 * @param[in] flags Flags for the frame. These are interchangable to the FCF
 *                  field definitons above, with the first byte for type and
 *                  and the second byte for addressing modes and frame
 *                  version.
 * @param[in] seq   Sequence number for frame.
 *
 * @return  Size of frame header on success.
 * @return  0, on error (flags set to unexpected state).
 */
size_t ieee802154_set_frame_hdr(uint8_t *buf, const ieee802154_addr_t *src,
                                const ieee802154_addr_t *dst,
                                network_uint16_t flags, uint8_t seq);

/**
 * @brief   Get length of MAC header.
 *
 * @todo include security header implications
 *
 * @param[in] mhr   MAC header.
 *
 * @return  Length of MAC header on success.
 * @return  0, on error (source mode or destination mode set to reserved).
 */
size_t ieee802154_get_frame_hdr_len(const uint8_t *mhr);

/**
 * @brief   Gets addresses from MAC header.
 *
 * @pre (@p src != NULL) && (@p dst != NULL)
 *
 * @param[in] mhr   MAC header.
 * @param[out] src  Source address in MAC header.
 * @param[out] dst  Destination address MAC header.
 *
 * @return  (IEEE802154_FCF_DST_ADDR_SHORT | IEEE802154_FCF_SRC_ADDR_SHORT),
 *          on success and if both addresses are short.
 * @return  (IEEE802154_FCF_DST_ADDR_SHORT | IEEE802154_FCF_SRC_ADDR_LONG),
 *          on success and if destination address is short and source is long.
 * @return  (IEEE802154_FCF_DST_ADDR_LONG | IEEE802154_FCF_SRC_ADDR_SHORT),
 *          on success and if destination address is long and source is short.
 * @return  (IEEE802154_FCF_DST_ADDR_LONG | IEEE802154_FCF_SRC_ADDR_LONG),
 *          on success and if destination address is long and source is long.
 * @return  etc.
 * @return  -EINVAL, if @p mhr contains unexpected flags.
 */
int ieee802154_get_addr(const uint8_t *mhr, ieee802154_addr_t *src,
                        ieee802154_addr_t *dst);

/**
 * @brief   Gets sequence number from MAC header.
 *
 * @pre length of allocated space at @p mhr > 3
 *
 * @param[in] mhr   MAC header.
 *
 * @return  The sequence number in @p mhr.
 */
static inline uint8_t ieee802154_get_seq(const uint8_t *mhr)
{
    return mhr[2];
}

/**
 * @brief   Generates an IPv6 interface identifier from an IEEE 802.15.4 address.
 *
 * @pre (@p eui64 != NULL) && (@p addr != NULL)
 * @see <a href="https://tools.ietf.org/html/rfc4944#section-6">
 *          RFC 4944, section 6
 *      </a>
 * @see <a href="https://tools.ietf.org/html/rfc6282#section-3.2.2">
 *          RFC 6282, section 3.2.2
 *      </a>
 *
 * @param[out] eui64    The resulting EUI-64.
 * @param[in] addr      An IEEE 802.15.4 address.
 * @param[in] addr_len  The length of @p addr. Must be 2 (short address),
 *                      4 (PAN ID + short address), or 8 (long address).
 *
 * @return Copy of @p eui64 on success.
 * @return NULL, if @p addr_len was of illegal length.
 */
static inline eui64_t *ieee802154_get_iid(eui64_t *eui64, const uint8_t *addr,
                                          size_t addr_len)
{
    int i = 0;

    eui64->uint8[0] = eui64->uint8[1] = 0;

    switch (addr_len) {
        case 8:
            eui64->uint8[0] = addr[i++] ^ 0x02;
            eui64->uint8[1] = addr[i++];
            eui64->uint8[2] = addr[i++];
            eui64->uint8[3] = addr[i++];
            eui64->uint8[4] = addr[i++];
            eui64->uint8[5] = addr[i++];
            eui64->uint8[6] = addr[i++];
            eui64->uint8[7] = addr[i++];
            break;

        case 4:
            eui64->uint8[0] = addr[i++] ^ 0x02;
            eui64->uint8[1] = addr[i++];

        case 2:
            eui64->uint8[2] = 0;
            eui64->uint8[3] = 0xff;
            eui64->uint8[4] = 0xfe;
            eui64->uint8[5] = 0;
            eui64->uint8[6] = addr[i++];
            eui64->uint8[7] = addr[i++];
            break;

        default:
            return NULL;
    }

    return eui64;
}

#ifdef __cplusplus
}
#endif

#endif /* IEEE802154_H_ */
/** @} */
