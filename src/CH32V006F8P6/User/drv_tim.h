/**
 * @file drv_tim.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 タイマドライバ
 * @version 0.1
 * @date 2026-03-15
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_TIM_H
#define DRV_TIM_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

// -----------------------------------------------------------
// [コンパイルスイッチ]
// #define USE_TIM_IRQ

// -----------------------------------------------------------
__attribute__((always_inline)) static inline uint16_t drv_get_tim_cnt(void)
{
    return TIM_GetCounter(TIM1);
}

extern volatile uint32_t g_systick_cnt_ms;
__attribute__((always_inline)) static inline uint32_t drv_get_systick_cnt(void)
{
    return g_systick_cnt_ms;
}

// -----------------------------------------------------------
#define SW_TIMER_BUG_SIZE     32
typedef struct {
    bool is_cnt_start;        // カウント開始有無
    bool is_intervel;         // 周期タイマー有無(false=ワンショット、true=周期タイマー)
    uint16_t config_time_ms;  // 目標のタイマーカウントms
    uint16_t cnt_time_ms;     // 今現在のタイマーカウントms
} software_timer_config_t;

// -----------------------------------------------------------
void drv_tick_delay_ms(uint32_t ms);
bool soft_timer_start(uint16_t config_time_ms, bool is_intervel, uint8_t *p_timer_no);
void soft_timer_stop(uint8_t timer_no);
bool get_soft_timer_cnt_match(uint8_t timer_no);
void soft_timer_proc(void);
void drv_tim_init(uint16_t arr, uint16_t psc, uint16_t div);
void drv_systick_init(void);
#endif // DRV_TIM_H