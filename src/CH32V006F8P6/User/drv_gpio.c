/**
 * @file drv_gpio.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 GPIOドライバ
 * @version 0.1
 * @date 2026-03-15
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_gpio.h"

bool g_is_btn_on_flg = false;
// -----------------------------------------------------------
// [外部割り込み(EXTI0) 割り込みハンドラ]

void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void EXTI7_0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
    {
        g_is_btn_on_flg = true;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

// -----------------------------------------------------------

void drv_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    // [GPIO PDポート初期化]
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO | RCC_PB2Periph_GPIOD, ENABLE);

    // ------------------------------------------------------
    // PD0 ... 動作: GPIO外部割り込み(EXTI0)、内蔵プルアップ=有効
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // EXTI0 ... アクティブLOW
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // ------------------------------------------------------
}
