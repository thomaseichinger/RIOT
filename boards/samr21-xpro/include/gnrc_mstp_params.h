/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   board_samr21-xpro
 * @{
 *
 * @file
 * @brief     MSTP board specific configuration
 *
 * @author    Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifndef GNRC_MSTP_PARAMS_H_
#define GNRC_MSTP_PARAMS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name GNRC_MSTP configuration
 */
static const  gnrc_mstp_params_t gnrc_mstp_params[] =
    {
        {
            .uart = GNRC_MSTP_UART,
            .baudrate = GNRC_MSTP_BAUDRATE,
        },
    };
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* GNRC_MSTP_PARAMS_H_ */
/** @} */
