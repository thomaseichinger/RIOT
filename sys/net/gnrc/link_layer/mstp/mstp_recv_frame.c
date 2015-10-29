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
#include "periph/gpio.h"
#include "xtimer.h"
#include "mstp_internal.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

msg_t mstp_recv_frame_msg;
uint16_t mstp_recv_frame_data_index;
int i;

void mstp_receive_frame(void *arg, char data)
{
    // gpio_set(GPIO_PIN(PA, 16));

    gnrc_mstp_t *ctx = (gnrc_mstp_t *)arg;

    switch (ctx->state) {
        case MSTP_STATE_IDLE:
            if (data == MSTP_DATA_PREAMBLE_1) {
                ctx->state = MSTP_STATE_PREAMBLE;
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
            }
            break;
        case MSTP_STATE_PREAMBLE:
            if (data == MSTP_DATA_PREAMBLE_2) {
                ctx->state = MSTP_STATE_HEADER;
                /* set timeout to return to IDLE here */
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
            }
            else {
                ctx->state = MSTP_STATE_IDLE;
                xtimer_remove(&(ctx->timer_fa));
            }
            break;
        case MSTP_STATE_HEADER:
            DEBUG("hdri: %d\n", ctx->frame.hdr_index);
            if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_FRAME_TYPE) {
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.type = data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, 0xff);
                /* Remember if payload octets are encoded */
                /* TODO: This also includes proprietary frames */
                if (ctx->frame.type >= MSTP_FRAME_TYPE_EXT_DATA_EXP_REPLY) {
                    ctx->frame.encoded = 1;
                }
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
                break;
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_DEST_ADDR){
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.dst_addr = data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
                break;
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_SRC_ADDR){
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.src_addr = data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
                break;
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_LEN_1){
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.length = (data<<8);
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
                break;
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_LEN_2){
                ctx->state = MSTP_STATE_HEADER;
                ctx->frame.length |= data;
                ctx->frame.hdr_index++;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
                break;
            }
            else if (ctx->frame.hdr_index == MSTP_FRAME_INDEX_HDR_CRC) {
                ctx->state = MSTP_STATE_VALIDATE_HEADER;
                ctx->frame.hdr_index = 0;
                ctx->frame.header_crc = mstp_crc_header_update(data, ctx->frame.header_crc);
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
            }
            else {
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
                ctx->frame.header_crc = 0xff;
                xtimer_remove(&(ctx->timer_fa));
                break;
            }
        case MSTP_STATE_VALIDATE_HEADER:
            DEBUG("calc hdr_crc: %02x\n", ctx->frame.header_crc);
            if (ctx->frame.header_crc != 0x55) {
                puts("!HCRC");
                /* Bad CRC */
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
                xtimer_remove(&(ctx->timer_fa));
            }
            else if ((ctx->frame.dst_addr != ctx->ll_addr) && (ctx->frame.dst_addr != MSTP_BROADCAST_ADDR)) {
                /* Not for us */
                DEBUG("not for us");
                if (ctx->frame.length != 0) {
                    ctx->state = MSTP_STATE_SKIP_DATA;
                    xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                                   &(ctx->msg_fa), ctx->mac_pid);
                }
                else {
                    ctx->state = MSTP_STATE_IDLE;
                    ctx->frame.hdr_index = 0;
                    xtimer_remove(&(ctx->timer_fa));
                }
            }
            else if (ctx->frame.length == 0) {
                /* For us, no data, signal valid frame */
                DEBUG("for us but no data");
                ctx->frame.valid = 1;
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
                mstp_recv_frame_msg.type = MSTP_EV_RECEIVED_VALID_FRAME;
                xtimer_remove(&(ctx->timer_fa));
                msg_send(&mstp_recv_frame_msg, ctx->mac_pid);
            }
            else {
                DEBUG("DATA!!!!");
                /* Proceed to receive payload */
                ctx->state = MSTP_STATE_DATA;
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
            }
            break;
        case MSTP_STATE_DATA:
            if (mstp_recv_frame_data_index < ctx->frame.length) {
                /* receive n=length bytes */
                ctx->frame.data[mstp_recv_frame_data_index++] = (uint8_t) data;
                ctx->frame.data_crc = mstp_crc_data_update(data, ctx->frame.data_crc);
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
                break;
            }
            else if (mstp_recv_frame_data_index == ctx->frame.length) {
                /* receive first crc octet */
                mstp_recv_frame_data_index++;
                ctx->frame.data_crc = (data<<8);
                // ctx->frame.data_crc = mstp_crc_data_update(data, ctx->frame.data_crc);
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
                break;
            }
            else if (mstp_recv_frame_data_index == (ctx->frame.length + 1)) {
                /* receive second CRC octet */
                // ctx->frame.data_crc = mstp_crc_data_update(data, ctx->frame.data_crc);
                ctx->frame.data_crc += data;
                ctx->state = MSTP_STATE_DATA_CRC;
                xtimer_remove(&(ctx->timer_fa));
            }
        case MSTP_STATE_DATA_CRC:
            if ((ctx->frame.data_crc) == 0xf0b8) {
                /* valid CRC, signal valid frame */
                ctx->frame.valid = 1;
                mstp_recv_frame_msg.type = MSTP_EV_RECEIVED_INVALID_FRAME;
            }
            else {
                DEBUG("ALARM!!!! WRONG DATA CRC!");
                DEBUG("was %04x\n", ctx->frame.data_crc);
                mstp_recv_frame_msg.type = MSTP_EV_RECEIVED_VALID_FRAME;
            }
            ctx->state = MSTP_STATE_IDLE;
            ctx->frame.hdr_index = 0;
            ctx->frame.header_crc = 0xff;
            ctx->frame.data_crc = 0xffff;
            mstp_recv_frame_data_index = 0;
            mstp_recv_frame_msg.type = MSTP_EV_RECEIVED_INVALID_FRAME;
            msg_send(&mstp_recv_frame_msg, ctx->mac_pid);
            break;
        case MSTP_STATE_SKIP_DATA:
            if (mstp_recv_frame_data_index <= ctx->frame.length) {
                /* consume data not for us */
                mstp_recv_frame_data_index++;
                xtimer_set_msg(&(ctx->timer_fa), MSTP_T_FRAME_ABORT,
                               &(ctx->msg_fa), ctx->mac_pid);
            }
            else {
                /* done consuming */
                ctx->state = MSTP_STATE_IDLE;
                ctx->frame.hdr_index = 0;
                ctx->frame.header_crc = 0xff;
                mstp_recv_frame_data_index = 0;
                xtimer_remove(&(ctx->timer_fa));
            }
    }
    DEBUG("St: %02x Rx: %02x\n", ctx->state, data);
    // gpio_clear(GPIO_PIN(PA, 16));
    // printf("Rx: %02x i: %02x\n", data, i++);

}