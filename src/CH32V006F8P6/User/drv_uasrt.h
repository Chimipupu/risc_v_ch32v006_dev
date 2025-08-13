/**
 * @file drv_uasrt.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 USARTドライバ＆ラッパー＆API
 * @version 0.1
 * @date 2025-08-06
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef DRV_UASRT_H
#define DRV_UASRT_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

#define USART_RX_BUF_SIZE   128

bool hw_usart_get_byte(uint8_t *p_val);
void hw_usart_init(void);
void hw_usart_rx_data_print(void);
uint8_t hw_usart_get_char(void);

#endif // DRV_UASRT_H
