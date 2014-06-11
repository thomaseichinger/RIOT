/*
 * Copyright (C) 2013 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

#ifndef CPUCONF_H_
#define CPUCONF_H_

/**
 * @ingroup		cpu_stm32f1
 *
 * @{
 */

/**
 * @file
 * @brief		STM32f103 CPUconfiguration
 *
 * @author    Freie Universität Berlin, Computer Systems & Telematics, FeuerWhere project
 * @author		baar
 * @author    Alaeddine Weslati <alaeddine.weslati@intia.fr>
 * @version
 *
 *
 */
#define TRANSCEIVER_BUFFER_SIZE (3)

#define FEUERWARE_CONF_CPU_NAME			"stm32f103rey6"

/**
 * @name CPU capabilities
 * @{
 */
//#define FEUERWARE_CPU_LPC2387					1
#define FEUERWARE_CONF_CORE_SUPPORTS_TIME		1
/** @} */

/**
 * @name Stdlib configuration
 * @{
 */
#define __FOPEN_MAX__		4
#define __FILENAME_MAX__	12
/** @} */

/**
 * @name Kernel configuration
 * @{
 */
#ifndef KERNEL_CONF_STACKSIZE_DEFAULT
#define KERNEL_CONF_STACKSIZE_DEFAULT	512
#endif

#define KERNEL_CONF_STACKSIZE_IDLE		512

#define KERNEL_CONF_STACKSIZE_PRINTF_FLOAT  (4096)
#define KERNEL_CONF_STACKSIZE_PRINTF        (2048)

#define UART0_BUFSIZE                   (128)

/** @} */

/**
 * @name Compiler specifics
 * @{
 */
#define CC_CONF_INLINE					inline
#define CC_CONF_USED					__attribute__((used))
#define CC_CONF_NONNULL(...)			__attribute__((nonnull(__VA_ARGS__)))
#define CC_CONF_WARN_UNUSED_RESULT		__attribute__((warn_unused_result))
/** @} */

#define CPU_ID_LEN              (12)
#define CPU_ID_EUI64_START      (4)
#define CPU_ID_RADDR_START      (8)

/** @} */
#endif /* CPUCONF_H_ */
