/**
 * @file drv_tim.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 TIM(タイマー)ドライバ＆ラッパー＆API
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef DRV_TIM_H
#define DRV_TIM_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

inline __attribute__((always_inline)) uint16_t drv_get_tim_cnt(void)
{
    return TIM_GetCounter(TIM1);
}

void drv_tim_init(uint16_t arr, uint16_t psc);

#endif // DRV_TIM_H