/*
 * Copyright (C) 2014 Loci Controls Inc.
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     driver_periph
 * @{
 *
 * @file        gpio.c
 * @brief       Low-level GPIO driver implementation
 *
 * @author      Ian Martin <ian@locicontrols.com>
 *
 * @}
 */

#include <stdint.h>

#include "cpu.h"
#include "cc2538-gpio.h"
#include "ioc.h"
#include "periph/gpio.h"
#include "periph_conf.h"

/* guard file in case no GPIO devices are defined */
#if GPIO_NUMOF

/**
 * @brief Generate a bit mask in which only the specified bit is high.
 *
 * @param[in] n Number of the bit to set high in the mask.
 *
 * @return A bit mask in which bit n is high.
*/
#define BIT(n) ( 1 << (n) )

/**
 * @brief Checks a bit in enable_lut to determine if a GPIO is enabled
 *
 * @param[in] dev RIOT GPIO device number
 *
 * @return 0 or 1 indicating if the specified GPIO is enabled
*/
#define gpio_enabled(dev) ( (enable_lut >> (dev)) & 1 )

typedef struct {
    gpio_cb_t cb;       /**< callback called from GPIO interrupt */
    void *arg;          /**< argument passed to the callback */
} gpio_state_t;

static gpio_state_t gpio_config[GPIO_NUMOF];

const uint32_t enable_lut = 0
#if GPIO_0_EN
    | BIT(0)
#endif
#if GPIO_1_EN
    | BIT(1)
#endif
#if GPIO_2_EN
    | BIT(2)
#endif
#if GPIO_3_EN
    | BIT(3)
#endif
#if GPIO_4_EN
    | BIT(4)
#endif
#if GPIO_5_EN
    | BIT(5)
#endif
#if GPIO_6_EN
    | BIT(6)
#endif
#if GPIO_7_EN
    | BIT(7)
#endif
#if GPIO_8_EN
    | BIT(8)
#endif
#if GPIO_9_EN
    | BIT(9)
#endif
#if GPIO_10_EN
    | BIT(10)
#endif
#if GPIO_11_EN
    | BIT(11)
#endif
#if GPIO_12_EN
    | BIT(12)
#endif
#if GPIO_13_EN
    | BIT(13)
#endif
#if GPIO_14_EN
    | BIT(14)
#endif
#if GPIO_15_EN
    | BIT(15)
#endif
#if GPIO_16_EN
    | BIT(16)
#endif
#if GPIO_17_EN
    | BIT(17)
#endif
#if GPIO_18_EN
    | BIT(18)
#endif
#if GPIO_19_EN
    | BIT(19)
#endif
#if GPIO_20_EN
    | BIT(20)
#endif
#if GPIO_21_EN
    | BIT(21)
#endif
#if GPIO_22_EN
    | BIT(22)
#endif
#if GPIO_23_EN
    | BIT(23)
#endif
#if GPIO_24_EN
    | BIT(24)
#endif
#if GPIO_25_EN
    | BIT(25)
#endif
#if GPIO_26_EN
    | BIT(26)
#endif
#if GPIO_27_EN
    | BIT(27)
#endif
#if GPIO_28_EN
    | BIT(28)
#endif
#if GPIO_29_EN
    | BIT(29)
#endif
#if GPIO_30_EN
    | BIT(30)
#endif
#if GPIO_31_EN
    | BIT(31)
#endif
;

const unsigned int pin_lut[] = {
#if GPIO_0_EN
    [ 0] = GPIO_0_PIN,
#endif
#if GPIO_1_EN
    [ 1] = GPIO_1_PIN,
#endif
#if GPIO_2_EN
    [ 2] = GPIO_2_PIN,
#endif
#if GPIO_3_EN
    [ 3] = GPIO_3_PIN,
#endif
#if GPIO_4_EN
    [ 4] = GPIO_4_PIN,
#endif
#if GPIO_5_EN
    [ 5] = GPIO_5_PIN,
#endif
#if GPIO_6_EN
    [ 6] = GPIO_6_PIN,
#endif
#if GPIO_7_EN
    [ 7] = GPIO_7_PIN,
#endif
#if GPIO_8_EN
    [ 8] = GPIO_8_PIN,
#endif
#if GPIO_9_EN
    [ 9] = GPIO_9_PIN,
#endif
#if GPIO_10_EN
    [10] = GPIO_10_PIN,
#endif
#if GPIO_11_EN
    [11] = GPIO_11_PIN,
#endif
#if GPIO_12_EN
    [12] = GPIO_12_PIN,
#endif
#if GPIO_13_EN
    [13] = GPIO_13_PIN,
#endif
#if GPIO_14_EN
    [14] = GPIO_14_PIN,
#endif
#if GPIO_15_EN
    [15] = GPIO_15_PIN,
#endif
#if GPIO_16_EN
    [16] = GPIO_16_PIN,
#endif
#if GPIO_17_EN
    [17] = GPIO_17_PIN,
#endif
#if GPIO_18_EN
    [18] = GPIO_18_PIN,
#endif
#if GPIO_19_EN
    [19] = GPIO_19_PIN,
#endif
#if GPIO_20_EN
    [20] = GPIO_20_PIN,
#endif
#if GPIO_21_EN
    [21] = GPIO_21_PIN,
#endif
#if GPIO_22_EN
    [22] = GPIO_22_PIN,
#endif
#if GPIO_23_EN
    [23] = GPIO_23_PIN,
#endif
#if GPIO_24_EN
    [24] = GPIO_24_PIN,
#endif
#if GPIO_25_EN
    [25] = GPIO_25_PIN,
#endif
#if GPIO_26_EN
    [26] = GPIO_26_PIN,
#endif
#if GPIO_27_EN
    [27] = GPIO_27_PIN,
#endif
#if GPIO_28_EN
    [28] = GPIO_28_PIN,
#endif
#if GPIO_29_EN
    [29] = GPIO_29_PIN,
#endif
#if GPIO_30_EN
    [30] = GPIO_30_PIN,
#endif
#if GPIO_31_EN
    [31] = GPIO_31_PIN,
#endif
};

static const uint32_t ioc_mask_lut[] = {
    [GPIO_NOPULL  ] = IOC_OVERRIDE_OE,
    [GPIO_PULLUP  ] = IOC_OVERRIDE_OE | IOC_OVERRIDE_PUE,
    [GPIO_PULLDOWN] = IOC_OVERRIDE_OE | IOC_OVERRIDE_PDE,
};

int gpio_init_out(gpio_t dev, gpio_pp_t pushpull)
{
    int pin;

    if (!gpio_enabled(dev)) {
        return -1;
    }

    pin = pin_lut[dev];
    gpio_software_control(pin);
    gpio_dir_output(pin);

    /* configure the pin's pull resistor state */
    IOC_PXX_OVER[pin] = ioc_mask_lut[pushpull];

    return 0;
}

int gpio_init_in(gpio_t dev, gpio_pp_t pushpull)
{
    int pin;

    if (!gpio_enabled(dev)) {
        return -1;
    }

    pin = pin_lut[dev];
    gpio_software_control(pin);
    gpio_dir_input(pin);

    /* configure the pin's pull resistor state */
    IOC_PXX_OVER[pin] = ioc_mask_lut[pushpull];

    return 0;
}

int gpio_init_int(gpio_t dev, gpio_pp_t pullup, gpio_flank_t flank, gpio_cb_t cb, void *arg)
{
    /* TODO: implement */
    (void)gpio_config; /**< Avoid compiler warning */
    return -1;
}

void gpio_irq_enable(gpio_t dev)
{
    /* TODO: implement */
}

void gpio_irq_disable(gpio_t dev)
{
    /* TODO: implement */
}

int gpio_read(gpio_t dev)
{
    int pin;

    if (!gpio_enabled(dev)) {
        return -1;
    }

    pin = pin_lut[dev];
    return (GPIO_NUM_TO_DEV(pin)->DATA >> GPIO_BIT_NUM(pin)) & 1;
}

void gpio_set(gpio_t dev)
{
    int pin;

    if (!gpio_enabled(dev)) {
        return;
    }

    pin = pin_lut[dev];
    GPIO_NUM_TO_DEV(pin)->DATA |= GPIO_PIN_MASK(GPIO_BIT_NUM(pin));
}

void gpio_clear(gpio_t dev)
{
    int pin;

    if (!gpio_enabled(dev)) {
        return;
    }

    pin = pin_lut[dev];
    GPIO_NUM_TO_DEV(pin)->DATA &= ~GPIO_PIN_MASK(GPIO_BIT_NUM(pin));
}

void gpio_toggle(gpio_t dev)
{
    if (gpio_read(dev)) {
        gpio_clear(dev);
    }
    else {
        gpio_set(dev);
    }
}

void gpio_write(gpio_t dev, int value)
{
    if (value) {
        gpio_set(dev);
    }
    else {
        gpio_clear(dev);
    }
}

#endif /* GPIO_NUMOF */
