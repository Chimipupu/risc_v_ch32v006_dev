/**
 * @file drv_gpio.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 GPIOドライバ
 * @version 0.1
 * @date 2026-03-15
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
extern bool g_is_btn_on_flg;

// -----------------------------------------------------------
void drv_gpio_init(void);

#endif // DRV_GPIO_H