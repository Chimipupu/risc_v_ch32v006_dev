/**
 * @file drv_tim.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 TIM(タイマー)ドライバ＆ラッパー＆API
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "drv_tim.h"

bool g_is_tim_cnt_up = false;

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/**
 * @brief TIM1カウントアップ割り込み
 * 
 */
void TIM1_UP_IRQHandler(void)
{
    ITStatus ret;

    ret = TIM_GetITStatus(TIM1, TIM_IT_Update);

    if(ret == SET) {
        g_is_tim_cnt_up = true;
    }

    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}

/**
 * @brief Initializes TIM1 output compare.
 * 
 * @param arr the period value.
 * @param psc the prescaler value.
 */
void drv_tim_init(uint16_t arr, uint16_t psc)
{
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    // -----------------------------------------------------------
    // TIM1 @1us周期、カウントアップ @65.535ms
    // -----------------------------------------------------------
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_TIM1, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 50;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
    // -----------------------------------------------------------

    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

#if 1
    TIM_Cmd(TIM1, ENABLE);
#else
    TIM_GenerateEvent(TIM1, TIM_IT_Update);
#endif
}