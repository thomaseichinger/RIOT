/*
 * Copyright (C) 2017 Thomas Eichinger
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ieee802154_mac_cmd IEEE802.15.4 MAC Commands
 * @ingroup     net_ieee802154
 * @brief       IEEE802.15.4 header definitions and utility functions
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 MAC command header definitions
 *
 * @author      Thomas Eichinger <thomas@riot-os.org>
 */

#ifndef IEEE802154_MACCMD_H
#define IEEE802154_MACCMD_H

#include <stdint.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief IEEE 802.15.4 MAC commands
 * @see IEEE Std 802.15.4-2015, 7.5.1 Command ID field
 * @{
 */
#define IEEE802154_MACCMD_ASS_REQ           (0x01)      /**< Association Request command */
#define IEEE802154_MACCMD_ASS_RSP           (0x02)      /**< Association Response command */
#define IEEE802154_MACCMD_DISASS_NOTI       (0x03)      /**< Disassociation Notification command */
#define IEEE802154_MACCMD_DATA_REQ          (0x04)      /**< Data Request command */
#define IEEE802154_MACCMD_PANID_CONFL_NOTI  (0x05)      /**< PAN ID Conflict Notification command */
#define IEEE802154_MACCMD_ORPHAN_NOTI       (0x06)      /**< Orphan Noitfication command */
#define IEEE802154_MACCMD_BEACON_REQ        (0x07)      /**< Beacon Request command */
#define IEEE802154_MACCMD_COORD_REALIGN     (0x08)      /**< Coordinator realignment command */
#define IEEE802154_MACCMD_GTS_REQ           (0x09)      /**< GTS request command */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* IEEE802154_H */
/** @} */