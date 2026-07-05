/**
 * @file drv_gpio.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 GPIOドライバ
 * @version 0.1
 * @date 2026-06-27
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_GPIO_H
#define DRV_GPIO_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

// -----------------------------------------------------------

#define GPIO_PORT_A_OFFSET    0x00
#define GPIO_PORT_B_OFFSET    0x10
#define GPIO_PORT_C_OFFSET    0x20
#define GPIO_PORT_D_OFFSET    0x30

typedef enum {
    // PA
    GPIO_PORT_A_0 = GPIO_PORT_A_OFFSET,
    GPIO_PORT_A_1,
    GPIO_PORT_A_2,
    GPIO_PORT_A_3,
    GPIO_PORT_A_4,
    GPIO_PORT_A_5,
    GPIO_PORT_A_6,
    GPIO_PORT_A_7,

    // PB
    GPIO_PORT_B_0 = GPIO_PORT_B_OFFSET,
    GPIO_PORT_B_1,
    GPIO_PORT_B_2,
    GPIO_PORT_B_3,
    GPIO_PORT_B_4,
    GPIO_PORT_B_5,
    GPIO_PORT_B_6,
    GPIO_PORT_B_7,

    // PC
    GPIO_PORT_C_0 = GPIO_PORT_C_OFFSET,
    GPIO_PORT_C_1,
    GPIO_PORT_C_2,
    GPIO_PORT_C_3,
    GPIO_PORT_C_4,
    GPIO_PORT_C_5,
    GPIO_PORT_C_6,
    GPIO_PORT_C_7,

    // PD
    GPIO_PORT_D_0 = GPIO_PORT_D_OFFSET,
    GPIO_PORT_D_1,
    GPIO_PORT_D_2,
    GPIO_PORT_D_3,
    GPIO_PORT_D_4,
    GPIO_PORT_D_5,
    GPIO_PORT_D_6,
    GPIO_PORT_D_7,
} GPIO_PORT;

extern bool g_is_btn_on_flg;

// -----------------------------------------------------------
// [API]
void drv_gpio_init(void);
void drv_gpio_port_onoff(uint8_t gpio_pin, uint8_t pin_val);

// -----------------------------------------------------------

#endif // DRV_GPIO_H