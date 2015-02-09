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

#ifndef GET_VALUES_H
#define GET_VALUES_H

#define ENABLE_DEBUG    (0)
#include "debug.h"

void get_values_init(void);
void send_msg_get_values(msg_t *m);
extern char get_values_stack_buffer[KERNEL_CONF_STACKSIZE_MAIN];
extern kernel_pid_t get_values_pid;

#endif /* GET_VALUES_H */
