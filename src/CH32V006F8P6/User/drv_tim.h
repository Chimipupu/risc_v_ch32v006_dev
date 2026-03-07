/**
 * @file drv_tim.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 タイマドライバ
 * @version 0.1
 * @date 2026-03-05
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_TIM_H
#define DRV_TIM_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

__attribute__((always_inline)) static inline uint16_t drv_get_tim_cnt(void)
{
    return TIM_GetCounter(TIM1);
}

__attribute__((always_inline)) static inline uint32_t drv_get_systick_cnt(void)
{
    return SysTick->CNT;
}

void drv_tim_init(uint16_t arr, uint16_t psc, uint16_t div);

#endif // DRV_TIM_H