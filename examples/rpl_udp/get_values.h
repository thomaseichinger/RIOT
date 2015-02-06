/*
 * Copyright (C)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup examples
 * @{
 *
 * @file
 * @brief
 *
 * @author
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "vtimer.h"
#include "thread.h"
#include "net_if.h"
#include "sixlowpan.h"
#include "udp.h"
#include "rpl.h"
#include "rpl/rpl_dodag.h"
#include "rpl_udp.h"
#include "transceiver.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

char get_values_stack_buffer[KERNEL_CONF_STACKSIZE_MAIN];
kernel_pid_t get_values_pid;

void *get_values(void *arg);
