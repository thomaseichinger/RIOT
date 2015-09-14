/*
 * Copyright (C) 2015 Sebastian Sontberg <sebastian@sontberg.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_xmc1000
 * @{
 *
 * @file
 * @brief           CPU specific definitions for internal peripheral handling
 *
 * @author          Sebastian Sontberg <sebastian@sontberg.de>
 *
 * @}
 */

#ifndef CPU_PERIPH_H_
#define CPU_PERIPH_H_

#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Mandatory function for defining a GPIO pin
 */
#define HAVE_GPIO_T
typedef uint8_t gpio_t;

#define GPIO(Port, Pin)    (Port | Pin)
#define GPIO_UNDEF         (0xff)

/**
 * @brief   Available PORTS on xmc1100
 */
enum {
    P0 = 0,
    P1 = 0x10,
    P2 = 0x20
};

#define HAVE_GPIO_DIR_T
typedef enum {
    GPIO_DIR_IN      = 0,         /**< configure pin as input */
    GPIO_DIR_IN_INV  = 4,         /**< configure pin as inverted input */
    GPIO_DIR_OUT     = 16,        /**< configure pin as output */
} gpio_dir_t;


/* These are for drivers that need to set an output pin to the output
 * of a peripheral. They are meant to be OR'd with gpio_dir_t. See
 * Table 17-5 in XMC1100 Reference Manual */
typedef enum {
    GPIO_ALT_OUT_1   = 1,
    GPIO_ALT_OUT_2   = 2,
    GPIO_ALT_OUT_3   = 3,
    GPIO_ALT_OUT_4   = 4,
    GPIO_ALT_OUT_5   = 5,
    GPIO_ALT_OUT_6   = 6,
    GPIO_ALT_OUT_7   = 7,
} gpio_alt_output_t;

#define HAVE_GPIO_PP_T
typedef enum {
    GPIO_NOPULL   = 0,     /**< do not use internal pull resistors */
    GPIO_PULLDOWN = 1,     /**< enable internal pull-down resistor */
    GPIO_PULLUP   = 2,     /**< enable internal pull-up resistor */
} gpio_pp_t;

#define HAVE_GPIO_FLANK_T
typedef enum {
    GPIO_RISING = 1,        /**< emit interrupt on rising flank */
    GPIO_FALLING = 2,       /**< emit interrupt on falling flank */
    GPIO_BOTH = 3           /**< emit interrupt on both flanks */
} gpio_flank_t;

typedef struct {
    USIC_CH_TypeDef *usic;   /**< pointer to USIC/Channel */
    gpio_t tx;               /**< transmit pin */
    gpio_t rx;               /**< receive pin */
    uint8_t dsel;            /**< data selection for input stage DX0 */
    gpio_alt_output_t asel;  /**< alternative function selection for tx pin */
} uart_conf_t;

/**
 * @brief   Definitions for the Event Request Unit (ERU)
 */
#define A1 (2)
#define B0 (0)
#define B1 (1)

#define ERU(No, Input) {No, Input}

typedef struct {
    unsigned exs: 2;
    unsigned connection: 2;
} eru_input_t;


#ifdef __cplusplus
}
#endif

#endif /* CPU_PERIPH_H_ */
