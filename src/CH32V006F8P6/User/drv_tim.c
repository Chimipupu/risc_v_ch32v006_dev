/**
 * @file drv_tim.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 タイマドライバ
 * @version 0.1
 * @date 2026-03-05
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_tim.h"

// -----------------------------------------------------------
typedef struct {
    uint8_t div;
    uint16_t div_config_val;
} drv_tim_div_t;

volatile const drv_tim_div_t g_tim_div_tbl[] = {
    {1, TIM_CKD_DIV1},
    {2, TIM_CKD_DIV2},
    {4, TIM_CKD_DIV4},
};
volatile const uint8_t g_tim_div_tbl_cnt = sizeof(g_tim_div_tbl) / sizeof(g_tim_div_tbl[0]);

// -----------------------------------------------------------
// [割り込みハンドラ]

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/**
 * @brief TIM1カウントアップ割り込みハンドラ
 */
void TIM1_UP_IRQHandler(void)
{
    ITStatus ret;
    ret = TIM_GetITStatus(TIM1, TIM_IT_Update);
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}

// -----------------------------------------------------------
// [ドライバ]

/**
 * @brief 16bitタイマ  TIM1初期化
 * @param arr 16bit カウント最大値
 * @param psc 16bit プリスケーラ
 * @param div 16bit 分周比(1分周:TIM_CKD_DIV1、2分周:TIM_CKD_DIV2、4分周: TIM_CKD_DIV4)
 */
void drv_tim_init(uint16_t arr, uint16_t psc, uint16_t div)
{
    uint8_t i;
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    volatile uint16_t div_config_val = TIM_CKD_DIV1; // デフォルトは1分周(TIM_CKD_DIV1)

    // -----------------------------------------------------------
    // [タイマ初期化]
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_TIM1, ENABLE);
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;

    // 分周比の設定値をテーブルから検索(1,2,4分周のいずれかを指定)
    for(i = 0; i < g_tim_div_tbl_cnt; i++)
    {
        if(g_tim_div_tbl[i].div == div) {
            div_config_val = g_tim_div_tbl[i].div_config_val;
            break;
        }
    }
    TIM_TimeBaseInitStructure.TIM_ClockDivision = div_config_val;

    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 50;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // -----------------------------------------------------------
    // [タイマ割り込み初期化]
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    // -----------------------------------------------------------

#if 1
    TIM_Cmd(TIM1, ENABLE);
#else
    TIM_GenerateEvent(TIM1, TIM_IT_Update);
#endif
}