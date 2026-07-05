/**
 * @file drv_gpio.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 GPIOドライバ
 * @version 0.1
 * @date 2026-06-27
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_gpio.h"

#include "pcb_board_define.h"
#ifdef USE_74HC595
// 自前の74HC595ドライバ (https://github.com/Chimipupu/drv_74hc595.git)
#include "drv_74hc595.h"
#endif

// -----------------------------------------------------------
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
// [API]

void drv_gpio_init(void)
{
    GPIO_InitTypeDef pd0_cfg = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    // [GPIO PDポート初期化]
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO | RCC_PB2Periph_GPIOD, ENABLE);

    // ------------------------------------------------------
    // PD0 ... 動作: GPIO外部割り込み(EXTI0)、内蔵プルアップ=有効
    pd0_cfg.GPIO_Pin = GPIO_Pin_0;
    pd0_cfg.GPIO_Mode = GPIO_Mode_IPU;
    pd0_cfg.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOD, &pd0_cfg);

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

#ifdef USE_74HC595
    GPIO_InitTypeDef pd2_cfg = {0}; // PD2: 74HC595 RCLKピン
    GPIO_InitTypeDef pd3_cfg = {0}; // PD3: 74HC595 SRCLKピン
    GPIO_InitTypeDef pd4_cfg = {0}; // PD4: 74HC595 SERピン

    pd2_cfg.GPIO_Pin = GPIO_Pin_2;
    pd2_cfg.GPIO_Mode = GPIO_Mode_Out_PP;
    pd2_cfg.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOD, &pd2_cfg);

    pd3_cfg.GPIO_Pin = GPIO_Pin_3;
    pd3_cfg.GPIO_Mode = GPIO_Mode_Out_PP;
    pd3_cfg.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOD, &pd3_cfg);

    pd4_cfg.GPIO_Pin = GPIO_Pin_4;
    pd4_cfg.GPIO_Mode = GPIO_Mode_Out_PP;
    pd4_cfg.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOD, &pd4_cfg);
#endif
}

void drv_gpio_port_onoff(uint8_t gpio_pin, uint8_t pin_val)
{
    GPIO_TypeDef *p_gpio_port;
    uint8_t pin_number;
    uint16_t pin_mask;

    // PA: 0~7
    if((gpio_pin >= GPIO_PORT_A_0) && (gpio_pin <= GPIO_PORT_A_7)) {
        p_gpio_port = GPIOA;
    }
    // PB: 8~15
    else if((gpio_pin >= GPIO_PORT_B_0) && (gpio_pin <= GPIO_PORT_B_7)) {
        p_gpio_port = GPIOB;
    }
    // PC: 16~23
    else if((gpio_pin >= GPIO_PORT_C_0) && (gpio_pin <= GPIO_PORT_C_7)) {
        p_gpio_port = GPIOC;
    }
    // PD: 24~31
    else if((gpio_pin >= GPIO_PORT_D_0) && (gpio_pin <= GPIO_PORT_D_7)) {
        p_gpio_port = GPIOD;
    }
    else {
        return;
    }

    pin_val = pin_val & 1;
    pin_number = gpio_pin & 0x07;
    pin_mask = (uint16_t)(1 << pin_number);
    GPIO_WriteBit(p_gpio_port, pin_mask, (pin_val == 0) ? Bit_RESET : Bit_SET);
}