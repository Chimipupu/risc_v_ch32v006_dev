/**
 * @file drv_uart.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 UARTドライバ
 * @version 0.1
 * @date 2026-03-10
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_UASRT_H
#define DRV_UASRT_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

bool drv_uart_get_char(uint8_t *p_data);
void drv_uart_init(void);

#endif // DRV_UASRT_H