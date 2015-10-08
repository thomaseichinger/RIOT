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
 * @brief       Recieve Frame FSM for the MS/TP MAC layer
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */


#include "msg.h"

#include "periph/uart.h"
#include "mstp_internal.h"
msg_t mstp_recv_frame_msg;
uint16_t mstp_recv_frame_data_index;


void mstp_receive_frame(void *arg, char data)
{
    gnrc_mstp_t *ctx = (gnrc_mstp_t *)arg;
    msg_t msg;
    printf("St: %02x Rx: %02x\n", ctx->state, data);
    switch (ctx->state) {
        case MSTP_STATE_IDLE:
            if (data == MSTP_DATA_PREAMBLE_1) {
                ctx->state = MSTP_STATE_PREAMBLE;
                // ctx->buffer[ctx->buffer_possition++] = data;
            }
            break;
        case MSTP_STATE_PREAMBLE:
            if (data == MSTP_DATA_PREAMBLE_1) {
                ctx->state = MSTP_STATE_PREAMBLE;
                /* set timeout to return to IDLE here */
            }
            else if (data == MSTP_DATA_PREAMBLE_2) {
                ctx->state = MSTP_STATE_HEADER;
                // ctx->buffer[ctx->buffer_possition++] = data;
            }
            else {
                ctx->state = MSTP_STATE_IDLE;
                // ctx->buffer_possition = 0;
            }
            break;
        case MSTP_STATE_HEADER:
            if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_FRAME_TYPE) {
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.type = data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
                /* Remember if payload octets are encoded */
                /* TODO: This also includes proprietary frames */
                if (ctx->frame.type >= MSTP_FRAME_TYPE_EXT_DATA_EXP_REPLY) {
                    ctx->frame.encoded = 1;
                }
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_DEST_ADDR){
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.dst_addr = data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_SRC_ADDR){
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.src_addr = data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_LEN_1){
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.length = (data<<8);
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_LEN_2){
                ctx->state = MSTP_STATE_HEADER_CRC;
                ctx->frame.length |= data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
            }
            else {
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
                ctx->frame.header_crc = 0xff;
            }
            /* HEADER CRC byte should be read in header_crc state */
            break;
        case MSTP_STATE_VALIDATE_HEADER:
            if (ctx->frame.header_crc != 0x55) {
                /* Bad CRC */
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
            }
            else if ((ctx->frame.dst_addr != ctx->addr) && (ctx->frame.dst_addr != MSTP_BROADCAST_ADDR)) {
                /* Not for us */
                if (ctx->frame.length != 0) {
                    ctx->state = MSTP_STATE_SKIP_DATA;
                }
                else {
                    ctx->state = MSTP_STATE_IDLE;
                    ctx->frame.hdr_index = 0;
                }
            }
            else if (ctx->frame.length == 0) {
                /* For us, no data, signal valid frame */
                ctx->frame.valid = 1;
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
                mstp_recv_frame_msg.type = MSTP_EV_RECEIVED_VALID_FRAME;
                msg_send(&msg, ctx->mac_pid);
            }
            else {
                /* Proceed to receive payload */
                ctx->state = MSTP_STATE_DATA;
            }
            break;
        case MSTP_STATE_DATA:
            if (mstp_recv_frame_data_index < ctx->frame.length) {
                /* receive n=length bytes */
                ctx->frame.data[mstp_recv_frame_data_index++] = (uint8_t) data;
                ctx->frame.data_crc = mstp_crc_data_update(data, ctx->frame.data_crc);
                break;
            }
            else if (mstp_recv_frame_data_index == ctx->frame.length) {
                /* receive first crc octet */
                ctx->frame.data_crc = mstp_crc_data_update(data, ctx->frame.data_crc);
                break;
            }
            else if (mstp_recv_frame_data_index == (ctx->frame.length + 1)) {
                /* receive second CRC octet */
                ctx->frame.data_crc = mstp_crc_data_update(data, ctx->frame.data_crc);
                ctx->state = MSTP_STATE_DATA_CRC;
            }
        case MSTP_STATE_DATA_CRC:
            if (ctx->frame.data_crc == 0xf0b8) {
                /* valid CRC, signal valid frame */
                ctx->frame.valid = 1;
            }
            ctx->state = MSTP_STATE_IDLE;
            ctx->frame.hdr_index = 0;
            ctx->frame.header_crc = 0xff;
            ctx->frame.data_crc = 0xffff;
            mstp_recv_frame_data_index = 0;
            mstp_recv_frame_msg.type = MSTP_EV_RECEIVED_VALID_FRAME;
            msg_send(&msg, ctx->mac_pid);
            break;
        case MSTP_STATE_SKIP_DATA:
            if (mstp_recv_frame_data_index <= ctx->frame.length) {
                /* consume data not for us */
                mstp_recv_frame_data_index++;
            }
            else {
                /* done consuming */
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
                ctx->frame.header_crc = 0xff;
                mstp_recv_frame_data_index = 0;
            }
    }
}