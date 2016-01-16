/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_at86rf2xx
 * @{
 *
 * @file
 * @brief       Netdev adaption for the AT86RF2xx drivers
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kévin Roussel <Kevin.Roussel@inria.fr>
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>

#include "net/eui64.h"
#include "net/ieee802154.h"
#include "net/netdev2.h"
#include "net/netdev2_ieee802154.h"
#include "net/gnrc/nettype.h"
#include "at86rf2xx.h"
#include "at86rf2xx_netdev.h"
#include "at86rf2xx_internal.h"
#include "at86rf2xx_registers.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define _MAX_MHR_OVERHEAD   (25)

static int _send(netdev2_t *netdev, struct iovec *vector, int count);
static int _recv(netdev2_t *netdev, char *buf, int len);
static int _init(netdev2_t *netdev);
static void _isr(netdev2_t *netdev);
static int _get(netdev2_t *netdev, netopt_t opt, void *val, size_t max_len);
static int _set(netdev2_t *netdev, netopt_t opt, void *val, size_t len);

const netdev2_driver_t at86rf2xx_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

static void _irq_handler(void *arg)
{
    netdev2_t *dev = (netdev2_t *) arg;

    if (dev->event_callback) {
        dev->event_callback(dev, NETDEV2_EVENT_ISR, NULL);
    }
}

static int _init(netdev2_t *netdev)
{
    at86rf2xx_t *dev = (at86rf2xx_t *)netdev;

    /* initialise GPIOs */
    gpio_init(dev->cs_pin, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_set(dev->cs_pin);
    gpio_init(dev->sleep_pin, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_clear(dev->sleep_pin);
    gpio_init(dev->reset_pin, GPIO_DIR_OUT, GPIO_NOPULL);
    gpio_set(dev->reset_pin);
    gpio_init_int(dev->int_pin, GPIO_NOPULL, GPIO_RISING, _irq_handler, dev);
    dev->rx_frame_len = 0;

    /* make sure device is not sleeping, so we can query part number */
    at86rf2xx_assert_awake(dev);

    /* test if the SPI is set up correctly and the device is responding */
    if (at86rf2xx_reg_read(dev, AT86RF2XX_REG__PART_NUM) !=
        AT86RF2XX_PARTNUM) {
        DEBUG("[at86rf2xx] error: unable to read correct part number\n");
        return -1;
    }

    /* reset device to default values and put it into RX state */
    at86rf2xx_reset(dev);

    return 0;
}

static int _send(netdev2_t *netdev, struct iovec *vector, int count)
{
    at86rf2xx_t *dev = (at86rf2xx_t *)netdev;
    const struct iovec *ptr = vector;
    size_t len = 0;
    uint8_t *mhr;

    assert(vector != NULL);
    assert(vector[0].iov_len > 3);

    for (int i = 0; i < count; i++, ptr++) {
        len += ptr->iov_len;
        /* len + 2: packet data + FCS */
        if ((len + 2) > AT86RF2XX_MAX_PKT_LENGTH) {
            printf("[at86rf2xx] error: packet too large (%u byte) to be send\n",
                   (unsigned)len + 2);
            return -EOVERFLOW;
        }
    }
    mhr = vector[0].iov_base;
    /* set internal sequence number */
    mhr[2] = dev->seq_nr++;
    ptr = vector;
    len = 0;        /* reset len and ptr as offset counter */

    at86rf2xx_tx_prepare(dev);

    /* load packet data into FIFO */
    for (int i = 0; i < count; i++, ptr++) {
        len = at86rf2xx_tx_load(dev, ptr->iov_base, ptr->iov_len, len);
    }

    /* send data out directly if pre-loading id disabled */
    if (!(dev->options & AT86RF2XX_OPT_PRELOADING)) {
        at86rf2xx_tx_exec(dev);
    }
    /* return the number of bytes that were actually send out */
    return (int)len;
}

static int _recv(netdev2_t *netdev, char *buf, int len)
{
    at86rf2xx_t *dev = (at86rf2xx_t *)netdev;
    uint8_t phr;
    size_t pkt_len;

    /* frame buffer protection will be unlocked as soon as at86rf2xx_fb_stop()
     * is called*/
    at86rf2xx_fb_start(dev);
    if (dev->rx_frame_len == 0) {

        /* get the size of the received packet */
        at86rf2xx_fb_read(dev, &phr, 1);

        /* Include FCS, LQI and RSSI in packet length */
        dev->rx_frame_len = phr + 2;

    }
    /* just return length when buf == NULL */
    if (buf == NULL) {
        at86rf2xx_fb_stop(dev);
        return dev->rx_frame_len;
    }
    /* transfer to internal variable and reset rx_frame_len for next packet */
    pkt_len = dev->rx_frame_len;
    dev->rx_frame_len = 0;
    /* not enough space in buf */
    if (pkt_len > len) {
        at86rf2xx_fb_stop(dev);
        return -ENOBUFS;
    }

#ifndef MODULE_AT86RF231
    at86rf2xx_fb_read(dev, (uint8_t *)buf, pkt_len);
    at86rf2xx_fb_stop(dev);
#else
    at86rf2xx_fb_read(dev, (uint8_t *)buf, pkt_len - 1);
    at86rf2xx_fb_stop(dev);
    /* append RSSI to packet */
    buf[pkt_len - 1] = at86rf2xx_reg_read(dev, AT86RF2XX_REG__PHY_ED_LEVEL);
#endif

    return pkt_len;
}

static int _set_state(at86rf2xx_t *dev, netopt_state_t state)
{
    switch (state) {
        case NETOPT_STATE_SLEEP:
            at86rf2xx_set_state(dev, AT86RF2XX_STATE_SLEEP);
            break;
        case NETOPT_STATE_IDLE:
            at86rf2xx_set_state(dev, AT86RF2XX_STATE_RX_AACK_ON);
            break;
        case NETOPT_STATE_TX:
            if (dev->options & AT86RF2XX_OPT_PRELOADING) {
                at86rf2xx_tx_exec(dev);
            }
            break;
        case NETOPT_STATE_RESET:
            at86rf2xx_reset(dev);
            break;
        default:
            return -ENOTSUP;
    }
    return sizeof(netopt_state_t);
}

netopt_state_t _get_state(at86rf2xx_t *dev)
{
    switch (at86rf2xx_get_status(dev)) {
        case AT86RF2XX_STATE_SLEEP:
            return NETOPT_STATE_SLEEP;
        case AT86RF2XX_STATE_BUSY_RX_AACK:
            return NETOPT_STATE_RX;
        case AT86RF2XX_STATE_BUSY_TX_ARET:
        case AT86RF2XX_STATE_TX_ARET_ON:
            return NETOPT_STATE_TX;
        case AT86RF2XX_STATE_RX_AACK_ON:
        default:
            return NETOPT_STATE_IDLE;
    }
}

static int _get(netdev2_t *netdev, netopt_t opt, void *val, size_t max_len)
{
    at86rf2xx_t *dev = (at86rf2xx_t *) netdev;

    if (netdev == NULL) {
        return -ENODEV;
    }

    /* getting these options doesn't require the transceiver to be responsive */
    switch (opt) {

        case NETOPT_ADDRESS:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = at86rf2xx_get_addr_short(dev);
            return sizeof(uint16_t);

        case NETOPT_ADDRESS_LONG:
            if (max_len < sizeof(uint64_t)) {
                return -EOVERFLOW;
            }
            *((uint64_t *)val) = at86rf2xx_get_addr_long(dev);
            return sizeof(uint64_t);

        case NETOPT_ADDR_LEN:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = 2;
            return sizeof(uint16_t);

        case NETOPT_SRC_LEN:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            if (dev->options & AT86RF2XX_OPT_SRC_ADDR_LONG) {
                *((uint16_t *)val) = 8;
            }
            else {
                *((uint16_t *)val) = 2;
            }
            return sizeof(uint16_t);

        case NETOPT_NID:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = dev->pan;
            return sizeof(uint16_t);

#ifdef MODULE_GNRC
        case NETOPT_PROTO:
            if (max_len < sizeof(gnrc_nettype_t)) {
                return -EOVERFLOW;
            }
            *((gnrc_nettype_t *)val) = dev->proto;
            return sizeof(gnrc_nettype_t);
#endif

        case NETOPT_CHANNEL:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            ((uint8_t *)val)[1] = 0;
            ((uint8_t *)val)[0] = at86rf2xx_get_chan(dev);
            return sizeof(uint16_t);

        case NETOPT_CHANNEL_PAGE:
            if (max_len < sizeof(uint16_t)) {
                return -EOVERFLOW;
            }
            ((uint8_t *)val)[1] = 0;
            ((uint8_t *)val)[0] = at86rf2xx_get_page(dev);
            return sizeof(uint16_t);

        case NETOPT_MAX_PACKET_SIZE:
            if (max_len < sizeof(int16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = AT86RF2XX_MAX_PKT_LENGTH - _MAX_MHR_OVERHEAD;
            return sizeof(uint16_t);

        case NETOPT_STATE:
            if (max_len < sizeof(netopt_state_t)) {
                return -EOVERFLOW;
            }
            *((netopt_state_t *)val) = _get_state(dev);
            return sizeof(netopt_state_t);

        case NETOPT_PRELOADING:
            if (dev->options & AT86RF2XX_OPT_PRELOADING) {
                *((netopt_enable_t *)val) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)val) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_AUTOACK:
            if (dev->options & AT86RF2XX_OPT_AUTOACK) {
                *((netopt_enable_t *)val) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)val) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_PROMISCUOUSMODE:
            if (dev->options & AT86RF2XX_OPT_PROMISCUOUS) {
                *((netopt_enable_t *)val) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)val) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_RAWMODE:
            if (dev->options & AT86RF2XX_OPT_RAWDUMP) {
                *((netopt_enable_t *)val) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)val) = NETOPT_DISABLE;
            }
            return sizeof(netopt_enable_t);

        case NETOPT_RX_START_IRQ:
            *((netopt_enable_t *)val) =
                !!(dev->options & AT86RF2XX_OPT_TELL_RX_START);
            return sizeof(netopt_enable_t);

        case NETOPT_RX_END_IRQ:
            *((netopt_enable_t *)val) =
                !!(dev->options & AT86RF2XX_OPT_TELL_RX_END);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_START_IRQ:
            *((netopt_enable_t *)val) =
                !!(dev->options & AT86RF2XX_OPT_TELL_TX_START);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_END_IRQ:
            *((netopt_enable_t *)val) =
                !!(dev->options & AT86RF2XX_OPT_TELL_TX_END);
            return sizeof(netopt_enable_t);

        case NETOPT_CSMA:
            *((netopt_enable_t *)val) =
                !!(dev->options & AT86RF2XX_OPT_CSMA);
            return sizeof(netopt_enable_t);

        default:
            /* Can still be handled in second switch */
            break;
    }

    int res;

    if (((res = netdev2_ieee802154_get(netdev, opt, val, max_len)) >= 0) ||
        (res != -ENOTSUP)) {
        return res;
    }

    uint8_t old_state = at86rf2xx_get_status(dev);
    res = 0;

    /* temporarily wake up if sleeping */
    if (old_state == AT86RF2XX_STATE_SLEEP) {
        at86rf2xx_assert_awake(dev);
    }

    /* these options require the transceiver to be not sleeping*/
    switch (opt) {
        case NETOPT_TX_POWER:
            if (max_len < sizeof(int16_t)) {
                res = -EOVERFLOW;
            }
            else {
                *((uint16_t *)val) = at86rf2xx_get_txpower(dev);
                res = sizeof(uint16_t);
            }
            break;

        case NETOPT_RETRANS:
            if (max_len < sizeof(uint8_t)) {
                res = -EOVERFLOW;
            }
            else {
                *((uint8_t *)val) = at86rf2xx_get_max_retries(dev);
                res = sizeof(uint8_t);
            }
            break;

        case NETOPT_IS_CHANNEL_CLR:
            if (at86rf2xx_cca(dev)) {
                *((netopt_enable_t *)val) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)val) = NETOPT_DISABLE;
            }
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_CSMA_RETRIES:
            if (max_len < sizeof(uint8_t)) {
                res = -EOVERFLOW;
            }
            else {
                *((uint8_t *)val) = at86rf2xx_get_csma_max_retries(dev);
                res = sizeof(uint8_t);
            }
            break;

        case NETOPT_CCA_THRESHOLD:
            if (max_len < sizeof(int8_t)) {
                res = -EOVERFLOW;
            }
            else {
                *((int8_t *)val) = at86rf2xx_get_cca_threshold(dev);
                res = sizeof(int8_t);
            }
            break;

        default:
            res = -ENOTSUP;
    }

    /* go back to sleep if were sleeping */
    if (old_state == AT86RF2XX_STATE_SLEEP) {
        at86rf2xx_set_state(dev, AT86RF2XX_STATE_SLEEP);
    }

    return res;
}

static int _set(netdev2_t *netdev, netopt_t opt, void *val, size_t len)
{
    at86rf2xx_t *dev = (at86rf2xx_t *) netdev;
    uint8_t old_state = at86rf2xx_get_status(dev);
    int res = 0;

    if (dev == NULL) {
        return -ENODEV;
    }

    /* temporarily wake up if sleeping */
    if (old_state == AT86RF2XX_STATE_SLEEP) {
        at86rf2xx_assert_awake(dev);
    }

    switch (opt) {
        case NETOPT_ADDRESS:
            if (len > sizeof(uint16_t)) {
                res = -EOVERFLOW;
            }
            else {
                at86rf2xx_set_addr_short(dev, *((uint16_t *)val));
                res = sizeof(uint16_t);
            }
            break;

        case NETOPT_ADDRESS_LONG:
            if (len > sizeof(uint64_t)) {
                res = -EOVERFLOW;
            }
            else {
                at86rf2xx_set_addr_long(dev, *((uint64_t *)val));
                res = sizeof(uint64_t);
            }
            break;

        case NETOPT_SRC_LEN:
            if (len > sizeof(uint16_t)) {
                res = -EOVERFLOW;
            }
            else {
                if (*((uint16_t *)val) == 2) {
                    at86rf2xx_set_option(dev, AT86RF2XX_OPT_SRC_ADDR_LONG,
                                         false);
                }
                else if (*((uint16_t *)val) == 8) {
                    at86rf2xx_set_option(dev, AT86RF2XX_OPT_SRC_ADDR_LONG,
                                         true);
                }
                else {
                    res = -ENOTSUP;
                    break;
                }
                res = sizeof(uint16_t);
            }
            break;

        case NETOPT_NID:
            if (len > sizeof(uint16_t)) {
                res = -EOVERFLOW;
            }
            else {
                at86rf2xx_set_pan(dev, *((uint16_t *)val));
                res = sizeof(uint16_t);
            }
            break;

#ifdef MODULE_GNRC
        case NETOPT_PROTO:
            if (len != sizeof(gnrc_nettype_t)) {
                res = -EINVAL;
            }
            else {
                dev->proto = *((gnrc_nettype_t *) val);
                res = sizeof(gnrc_nettype_t);
            }
            break;
#endif

        case NETOPT_CHANNEL:
            if (len != sizeof(uint16_t)) {
                res = -EINVAL;
            }
            else {
                uint8_t chan = ((uint8_t *)val)[0];
                if (chan < AT86RF2XX_MIN_CHANNEL ||
                    chan > AT86RF2XX_MAX_CHANNEL) {
                    res = -ENOTSUP;
                    break;
                }
                at86rf2xx_set_chan(dev, chan);
                res = sizeof(uint16_t);
            }
            break;

        case NETOPT_CHANNEL_PAGE:
            if (len != sizeof(uint16_t)) {
                res = -EINVAL;
            }
            else {
                uint8_t page = ((uint8_t *)val)[0];
#ifdef MODULE_AT86RF212B
                if ((page != 0) && (page != 2)) {
                    res = -ENOTSUP;
                }
                else {
                    at86rf2xx_set_page(dev, page);
                    res = sizeof(uint16_t);
                }
#else
                /* rf23x only supports page 0, no need to configure anything in the driver. */
                if (page != 0) {
                    res = -ENOTSUP;
                }
                else {
                    res = sizeof(uint16_t);
                }
#endif
            }
            break;

        case NETOPT_TX_POWER:
            if (len > sizeof(int16_t)) {
                res = -EOVERFLOW;
            }
            else {
                at86rf2xx_set_txpower(dev, *((int16_t *)val));
                res = sizeof(uint16_t);
            }
            break;

        case NETOPT_STATE:
            if (len > sizeof(netopt_state_t)) {
                res = -EOVERFLOW;
            }
            else {
                res = _set_state(dev, *((netopt_state_t *)val));
            }
            break;

        case NETOPT_AUTOACK:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_AUTOACK,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_RETRANS:
            if (len > sizeof(uint8_t)) {
                res = -EOVERFLOW;
            }
            else {
                at86rf2xx_set_max_retries(dev, *((uint8_t *)val));
                res = sizeof(uint8_t);
            }
            break;

        case NETOPT_PRELOADING:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_PRELOADING,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_PROMISCUOUSMODE:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_PROMISCUOUS,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_RAWMODE:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_RAWDUMP,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_RX_START_IRQ:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_TELL_RX_START,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_RX_END_IRQ:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_TELL_RX_END,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_TX_START_IRQ:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_TELL_TX_START,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_TX_END_IRQ:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_TELL_TX_END,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_CSMA:
            at86rf2xx_set_option(dev, AT86RF2XX_OPT_CSMA,
                                 ((bool *)val)[0]);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_CSMA_RETRIES:
            if ((len > sizeof(uint8_t)) ||
                (*((uint8_t *)val) > 5)) {
                res = -EOVERFLOW;
            }
            else if (!(dev->options & AT86RF2XX_OPT_CSMA)) {
                /* If CSMA is disabled, don't allow setting retries */
                res = -ENOTSUP;
            }
            else {
                at86rf2xx_set_csma_max_retries(dev, *((uint8_t *)val));
                res = sizeof(uint8_t);
            }
            break;

        case NETOPT_CCA_THRESHOLD:
            if (len > sizeof(int8_t)) {
                res = -EOVERFLOW;
            }
            else {
                at86rf2xx_set_cca_threshold(dev, *((int8_t *)val));
                res = sizeof(int8_t);
            }
            break;

        default:
            res = -ENOTSUP;
    }

    /* go back to sleep if were sleeping and state hasn't been changed */
    if ((old_state == AT86RF2XX_STATE_SLEEP) &&
        (opt != NETOPT_STATE)) {
        at86rf2xx_set_state(dev, AT86RF2XX_STATE_SLEEP);
    }

    if (res == -ENOTSUP) {
        res = netdev2_ieee802154_set(netdev, opt, val, len);
    }

    return res;
}

static void _isr(netdev2_t *netdev)
{
    at86rf2xx_t *dev = (at86rf2xx_t *) netdev;
    uint8_t irq_mask;
    uint8_t state;
    uint8_t trac_status;

    /* If transceiver is sleeping register access is impossible and frames are
     * lost anyway, so return immediately.
     */
    state = at86rf2xx_get_status(dev);
    if (state == AT86RF2XX_STATE_SLEEP) {
        return;
    }

    /* read (consume) device status */
    irq_mask = at86rf2xx_reg_read(dev, AT86RF2XX_REG__IRQ_STATUS);

    trac_status = at86rf2xx_reg_read(dev, AT86RF2XX_REG__TRX_STATE) &
                  AT86RF2XX_TRX_STATE_MASK__TRAC;

    if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__RX_START) {
        dev->event_callback(netdev, NETDEV2_EVENT_RX_STARTED, NULL);
        DEBUG("[at86rf2xx] EVT - RX_START\n");
    }

    if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__TRX_END) {
        if (state == AT86RF2XX_STATE_RX_AACK_ON ||
            state == AT86RF2XX_STATE_BUSY_RX_AACK) {
            DEBUG("[at86rf2xx] EVT - RX_END\n");
            if (!(dev->options & AT86RF2XX_OPT_TELL_RX_END)) {
                return;
            }
            dev->event_callback(netdev, NETDEV2_EVENT_RX_COMPLETE, NULL);
        }
        else if (state == AT86RF2XX_STATE_TX_ARET_ON ||
                 state == AT86RF2XX_STATE_BUSY_TX_ARET) {
            at86rf2xx_set_state(dev, dev->idle_state);
            DEBUG("[at86rf2xx] EVT - TX_END\n");
            DEBUG("[at86rf2xx] return to state 0x%x\n", dev->idle_state);

            if (dev->event_callback && (dev->options & AT86RF2XX_OPT_TELL_TX_END)) {
                switch (trac_status) {
                    case AT86RF2XX_TRX_STATE__TRAC_SUCCESS:
                    case AT86RF2XX_TRX_STATE__TRAC_SUCCESS_DATA_PENDING:
                        dev->event_callback(netdev, NETDEV2_EVENT_TX_COMPLETE, NULL);
                        DEBUG("[at86rf2xx] TX SUCCESS\n");
                        break;
                    case AT86RF2XX_TRX_STATE__TRAC_NO_ACK:
                        dev->event_callback(netdev, NETDEV2_EVENT_TX_NOACK, NULL);
                        DEBUG("[at86rf2xx] TX NO_ACK\n");
                        break;
                    case AT86RF2XX_TRX_STATE__TRAC_CHANNEL_ACCESS_FAILURE:
                        dev->event_callback(netdev, NETDEV2_EVENT_TX_MEDIUM_BUSY, NULL);
                        DEBUG("[at86rf2xx] TX_CHANNEL_ACCESS_FAILURE\n");
                        break;
                    default:
                        DEBUG("[at86rf2xx] Unhandled TRAC_STATUS: %d\n",
                              trac_status >> 5);
                }
            }
        }
    }
}
