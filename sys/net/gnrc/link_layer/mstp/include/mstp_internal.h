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
 * @brief       Internal definitions for the MS/TP MAC layer
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef GNRC_MSTP_INTERNAL_H_
#define GNRC_MSTP_INTERNAL_H_

#include "periph/uart.h"
#include "net/gnrc.h"
#include "net/gnrc/mstp.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MSTP_FRAME_TYPE_TOKEN                       (0x00)
#define MSTP_FRAME_TYPE_POLL_FOR_MASTER             (0x01)
#define MSTP_FRAME_TYPE_REPLY_POLL_FOR_MASTER       (0x02)
#define MSTP_FRAME_TYPE_TEST_REQUEST                (0x03)
#define MSTP_FRAME_TYPE_TEST_RESPONSE               (0x04)
#define MSTP_FRAME_TYPE_DATA_EXP_REPLY              (0x05)
#define MSTP_FRAME_TYPE_DATA_NOT_EXP_REPLY          (0x06)
#define MSTP_FRAME_TYPE_REPLY_POSTPONED             (0x07)
#define MSTP_FRAME_TYPE_EXT_DATA_EXP_REPLY          (0x20)
#define MSTP_FRAME_TYPE_EXT_DATA_EXP_NO_REPLY       (0x21)

#define MSTP_BROADCAST_ADDR     (0xff)

#define MSTP_DATA_PREAMBLE_1    (0x55)
#define MSTP_DATA_PREAMBLE_2    (0xff)

#define MSTP_FRAME_INDEX_DEST_ADDR (0x03)
#define MSTP_FRAME_INDEX_SRC_ADDR  (0x04)
#define MSTP_FRAME_INDEX_LEN_1     (0x05)
#define MSTP_FRAME_INDEX_LEN_2     (0x06)
#define MSTP_FRAME_INDEX_FRAME_TYPE     (0x07)

#define MSTP_STATE_IDLE         (0x00)
#define MSTP_STATE_PREAMBLE     (0x01)
#define MSTP_STATE_HEADER       (0x02)
#define MSTP_STATE_HEADER_CRC   (0x07)
#define MSTP_STATE_VALIDATE_HEADER  (0x03)
#define MSTP_STATE_DATA         (0x04)
#define MSTP_STATE_DATA_CRC     (0x05)
#define MSTP_STATE_SKIP_DATA    (0x06)

#define MSTP_SLAVE_IDLE (0x00)
#define MSTP_SLAVE_ANSWER_DATA (0x01)

#define MSTP_EV_RECEIVED_VALID_FRAME            (0xaa)
#define MSTP_EV_SUCCESSFULL_RECEPTION           (0xbb)
#define MSTP_EV_SEND_FRAME                      (0xcc)

#define MSTP_T_SEND_WAIT        (600)   /**< interval to wait between bytes sent */



typedef struct
{
    uart_t uart;
    kernel_pid_t mac_pid;
    uint8_t state;
    mstp_frame_t frame;
    uint8_t addr;

} mstp_slave_t;

typedef struct
{
    uart_t uart;
    kernel_pid_t mac_pid;
    uint8_t state;
    uint8_t addr;
    mstp_frame_t frame;
} mstp_master_t;

int mstp_snipify_pkt(gnrc_mstp_t *ctx, gnrc_pktsnip_t *pkt);

void mstp_receive_frame(void *arg, char data);

uint8_t mstp_crc_header_update(uint8_t data, uint8_t crc_in);

uint16_t mstp_crc_data_update(uint8_t data, uint16_t crc_in);

#ifdef __cplusplus
}
#endif

#endif /* GNRC_MSTP_INTERNAL_H_ */
/** @} */