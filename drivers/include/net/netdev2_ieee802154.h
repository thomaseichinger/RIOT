/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup drivers_netdev_netdev2
 * @brief
 * @{
 *
 * @file
 * @brief   Definitions for netdev2 common IEEE 802.15.4 code
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */
#ifndef NETDEV2_IEEE802154_H_
#define NETDEV2_IEEE802154_H_

#include "net/netopt.h"
#include "net/netdev2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Fallback function for netdev2 IEEE 802.15.4 devices' _get function
 *
 * Supposed to be used by netdev2 drivers as default case.
 *
 * @pre     Driver *MUST* implement NETOPT_ADDRESS and NETOPT_ADDRESS case!
 *
 * @param[in]   dev     network device descriptor
 * @param[in]   opt     option type
 * @param[out]  value   pointer to store the option's value in
 * @param[in]   max_len maximal amount of byte that fit into @p value
 *
 * @return              number of bytes written to @p value
 * @return              <0 on error
 */
int netdev2_ieee802154_get(netdev2_t *dev, netopt_t opt, void *value, size_t max_len);

/**
 * @brief   Fallback function for netdev2 IEEE 802.15.4 devices' _set function
 *
 * @param[in] dev       network device descriptor
 * @param[in] opt       option type
 * @param[in] value     value to set
 * @param[in] value_len the length of @p value
 *
 * @return              number of bytes used from @p value
 * @return              <0 on error
 */
int netdev2_ieee802154_set(netdev2_t *dev, netopt_t opt, void *value, size_t value_len);

#ifdef __cplusplus
}
#endif

#endif /* NETDEV2_IEEE802154_H_ */
/** @} */
