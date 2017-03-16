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
 * @brief       CRC functions for the MS/TP MAC layer
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#include <stdint.h>

uint8_t mstp_crc_header_update(uint8_t data, uint8_t crc_in)
{
    uint16_t crc;

    crc = crc_in ^ data;

    crc = crc ^ (crc << 1) ^ (crc << 2) ^ (crc << 3)
        ^ (crc << 4) ^ (crc << 5) ^ (crc << 6)
        ^ (crc << 7);

    return (crc & 0xfe) ^ ((crc >> 8) & 1);
}

uint16_t mstp_crc_data_update(uint8_t data, uint16_t crc_in)
{
    uint16_t crc;

    crc = (crc_in & 0xff) ^ data;

    return (crc_in >> 8) ^ (crc << 8) ^ (crc << 3)
        ^ (crc << 12) ^ (crc >> 4) ^ (crc & 0x0f) 
        ^ ((crc & 0x0f) << 7);
}

/* See BACnet Addendum 135-2012an, section G.3.2 */
#define CRC32K_INITIAL_VALUE (0xFFFFFFFF)
#define CRC32K_RESIDUE (0x0843323B)

/* CRC-32K polynomial, 1 + x**1 + ... + x**30 (+ x**32) */
#define CRC32K_POLY (0xEB31D82E)

/*
* Accumulate 'data_value' into the CRC in 'crc_value'.
* Return updated CRC.
*
* Note: crcValue must be set to CRC32K_INITIAL_VALUE
* before initial call.
*/
uint32_t mstp_crc_enc_data_update(uint8_t data_value, uint32_t crc_value)
{
    int b;

    for (b = 0; b < 8; b++) {
        if ((data_value & 1) ^ (crc_value & 1)) {
            crc_value >>= 1;
            crc_value ^= CRC32K_POLY;
        } else {
            crc_value >>= 1;
        }
        data_value >>= 1;
    }
    return crc_value;
}