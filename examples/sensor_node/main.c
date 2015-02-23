/*
 * Copyright (C) 2014 Hamburg University of Applied Sciences
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 *
 * @file
 * @brief       Example application for Embedded World 2015
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#include <stdio.h>
#include <string.h>

#include "vtimer.h"
#include "periph/adc.h"
#include "periph/gpio.h"

#include "thread.h"

#include "ieee802154_frame.h"
#include "transceiver.h"


#define RES                         ADC_RES_12BIT
#define ADC_IN_USE                  ADC_0
#define ADC_CHANNEL_USE             0
#define GPIO_POWER_PIN              GPIO_0

#define SND_BUFFER_SIZE     (100)
#define RCV_BUFFER_SIZE     (64)
#define RADIO_STACK_SIZE    (KERNEL_CONF_STACKSIZE_DEFAULT)

#define SENSOR_NODE_DISTANCE_TYPE 0x55

static msg_t msg_q[RCV_BUFFER_SIZE];

#define SLEEP       (1000 * 1000U)

static char text_msg[20];


void init_transceiver(void)
{
    uint16_t transceivers = TRANSCEIVER_DEFAULT;

    transceiver_init(transceivers);
    (void) transceiver_start();
    transceiver_register(transceivers, thread_getpid());
}

void transceiver_send_handler(uint16_t dest)
{
    if (transceiver_pid == KERNEL_PID_UNDEF) {
        puts("Transceiver not initialized");
        return;
    }

    ieee802154_packet_t p;

    transceiver_command_t tcmd;
    tcmd.transceivers = TRANSCEIVER_AT86RF231;
    tcmd.data = &p;

    memset(&p, 0, sizeof(ieee802154_packet_t));
    p.frame.payload = (uint8_t*) text_msg;
    p.frame.payload_len = strlen(text_msg) + 1;
    p.frame.fcf.frame_type = IEEE_802154_DATA_FRAME;
    p.frame.fcf.dest_addr_m = IEEE_802154_SHORT_ADDR_M;
    p.frame.fcf.src_addr_m = IEEE_802154_SHORT_ADDR_M;
    p.frame.dest_addr[1] = (dest&0xff);
    p.frame.dest_addr[0] = (dest>>8);
    p.frame.dest_pan_id = SENSOR_NODE_PAN_ID;

    msg_t mesg;
    mesg.type = SND_PKT;
    mesg.content.ptr = (char *) &tcmd;

    printf("[transceiver] Sending packet of length %" PRIu16 " to %" PRIu16 ": %s\n", p.frame.payload_len, p.frame.dest_addr[1], (char*) p.frame.payload);

    msg_send_receive(&mesg, &mesg, transceiver_pid);
    int8_t response = mesg.content.value;
    printf("[transceiver] Packet sent: %" PRIi8 "\n", response);
}

int main(void)
{
    msg_t m;

    timex_t sleep1 = timex_set(1, 0); /* 1 sec. */

    /* initialize a GPIO that powers the sensor just during a measure */
    printf("Initializing GPIO_%i as power supplying pin", GPIO_POWER_PIN);
    if (gpio_init_out(GPIO_POWER_PIN, GPIO_NOPULL) == 0) {
        puts("    ...[ok]");
    }
    else {
        puts("    ...[failed]");
        return 1;
    }
    puts("\n");

    /* initialize ADC device */
    printf("Initializing ADC_%i @ %i bit resolution", ADC_IN_USE, (6 + (2* RES)));
    if (adc_init(ADC_IN_USE, RES) == 0) {
        puts("    ...[ok]");
    }
    else {
        puts("    ...[failed]");
        return 1;
    }
    puts("\n");


    init_transceiver();


    msg_init_queue(msg_q, RCV_BUFFER_SIZE);

    memset(text_msg, 0xaa, 20);
    text_msg[0] = SENSOR_NODE_SRC_ID;
    text_msg[1] = SENSOR_NODE_DISTANCE_TYPE;

    while(1) {
        msg_try_receive(&m);
        if (m.type == PKT_PENDING) {
            ((ieee802154_packet_t*) m.content.ptr)->processing--;

        }
        gpio_set(GPIO_POWER_PIN);
        vtimer_sleep(sleep1);
        uint16_t sample_val = adc_sample(ADC_IN_USE, ADC_CHANNEL_USE);
        printf("sample_val: %i\n", sample_val);
        text_msg[2] = (sample_val>>8);
        text_msg[3] = (char)(sample_val&0xff);
        transceiver_send_handler(CENTRAL_NODE_ADDR);


         /* wait for next measure */
        vtimer_usleep(SLEEP);
    }
}