/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/*
 * @ingroup auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief   Auto initialization for MS/TP RS485 network interfaces
 *
 * @author  Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

#ifdef MODULE_MSTP

#include "board.h"
#include "net/gnrc/mstp.h"
#include "net/gnrc.h"

#include "gnrc_mstp_params.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Define stack parameters for the MAC layer thread
 * @{
 */
#define GNRC_MSTP_MAC_STACKSIZE     (THREAD_STACKSIZE_DEFAULT)
#define GNRC_MSTP_MAC_PRIO          (THREAD_PRIORITY_MAIN - 1)

#define GNRC_MSTP_NUM (sizeof(gnrc_mstp_params)/sizeof(gnrc_mstp_params[0]))

static gnrc_mstp_t gnrc_mstp_devs[GNRC_MSTP_NUM];
static char _mstp_stacks[GNRC_MSTP_MAC_STACKSIZE][GNRC_MSTP_NUM];

void auto_init_gnrc_mstp(void)
{
    for (int i = 0; i < GNRC_MSTP_NUM; i++) {
        DEBUG("Initializing MS/TP on UART_%i\n", i);
        const gnrc_mstp_params_t *params = &gnrc_mstp_params[i];
        gnrc_mstp_init((gnrc_mstp_t *)&gnrc_mstp_devs[i], params);
        gnrc_mstp_start(_mstp_stacks[i],
                        GNRC_MSTP_MAC_STACKSIZE, GNRC_MSTP_MAC_PRIO,
                        "mstp master", (gnrc_mstp_t *)&gnrc_mstp_devs[i]);
    }
}
#else
typedef int dont_be_pedantic;
#endif /* MODULE_MSTP */

/** @} */
