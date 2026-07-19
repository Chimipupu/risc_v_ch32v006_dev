/**
 * @file drv_gpio.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 GPIOドライバ
 * @version 0.1
 * @date 2026-07-05
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_gpio.h"
#include "pcb_board_define.h"

#ifdef USE_74HC595
// 自前の74HC595ドライバ (https://github.com/Chimipupu/drv_74hc595.git)
#include "drv_74hc595.h"
#endif // USE_74HC595

// -----------------------------------------------------------
#ifdef USE_GPIO_TBL
typedef struct {
    GPIO_PORT port_enum;
    GPIO_TypeDef *p_gpio_port;
    uint16_t pin_mask;
} drv_gpio_tbl_t;

static const drv_gpio_tbl_t drv_gpio_tbl[] = {
    // GPIO: PA 0~7
    {GPIO_PORT_A_0, GPIOA, (1u << 0)},
    {GPIO_PORT_A_1, GPIOA, (1u << 1)},
    {GPIO_PORT_A_2, GPIOA, (1u << 2)},
    {GPIO_PORT_A_3, GPIOA, (1u << 3)},
    {GPIO_PORT_A_4, GPIOA, (1u << 4)},
    {GPIO_PORT_A_5, GPIOA, (1u << 5)},
    {GPIO_PORT_A_6, GPIOA, (1u << 6)},
    {GPIO_PORT_A_7, GPIOA, (1u << 7)},

    // GPIO: PB 0~7
    {GPIO_PORT_B_0, GPIOB, (1u << 0)},
    {GPIO_PORT_B_1, GPIOB, (1u << 1)},
    {GPIO_PORT_B_2, GPIOB, (1u << 2)},
    {GPIO_PORT_B_3, GPIOB, (1u << 3)},
    {GPIO_PORT_B_4, GPIOB, (1u << 4)},
    {GPIO_PORT_B_5, GPIOB, (1u << 5)},
    {GPIO_PORT_B_6, GPIOB, (1u << 6)},
    {GPIO_PORT_B_7, GPIOB, (1u << 7)},

    // GPIO: PC 0~7
    {GPIO_PORT_C_0, GPIOC, (1u << 0)},
    {GPIO_PORT_C_1, GPIOC, (1u << 1)},
    {GPIO_PORT_C_2, GPIOC, (1u << 2)},
    {GPIO_PORT_C_3, GPIOC, (1u << 3)},
    {GPIO_PORT_C_4, GPIOC, (1u << 4)},
    {GPIO_PORT_C_5, GPIOC, (1u << 5)},
    {GPIO_PORT_C_6, GPIOC, (1u << 6)},
    {GPIO_PORT_C_7, GPIOC, (1u << 7)},

    // GPIO: PD 0~7
    {GPIO_PORT_D_0, GPIOD, (1u << 0)},
    {GPIO_PORT_D_1, GPIOD, (1u << 1)},
    {GPIO_PORT_D_2, GPIOD, (1u << 2)},
    {GPIO_PORT_D_3, GPIOD, (1u << 3)},
    {GPIO_PORT_D_4, GPIOD, (1u << 4)},
    {GPIO_PORT_D_5, GPIOD, (1u << 5)},
    {GPIO_PORT_D_6, GPIOD, (1u << 6)},
    {GPIO_PORT_D_7, GPIOD, (1u << 7)},
};
static const uint8_t GPIO_TBL_CNT =  (sizeof(drv_gpio_tbl) / sizeof(drv_gpio_tbl[0]));
#endif // USE_GPIO_TBL

#ifdef USE_BUTTON
bool g_is_btn_on_flg = false;
#endif // USE_BUTTON
// -----------------------------------------------------------
#ifdef USE_BUTTON
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
#endif // USE_BUTTON
// -----------------------------------------------------------
// [API]

void drv_gpio_init(void)
{
#ifdef USE_BUTTON
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
#endif // USE_BUTTON

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
#endif // USE_74HC595
}

void drv_gpio_port_onoff(uint8_t gpio_pin, uint8_t pin_val)
{
// テーブル検索方式: ROMを食う（最適化なしで408バイト）
#ifdef USE_GPIO_TBL
    uint8_t i;

    // テーブル検索
    for(i = 0; i < GPIO_TBL_CNT; i++)
    {
        if(drv_gpio_tbl[i].port_enum == gpio_pin) {
            if(pin_val == 0) {
                // GPIO -> Low
                drv_gpio_tbl[i].p_gpio_port->BCR = drv_gpio_tbl[i].pin_mask;
            } else {
                // GPIO -> High
                drv_gpio_tbl[i].p_gpio_port->BSHR = drv_gpio_tbl[i].pin_mask;
            }
        }
    }
#else
    GPIO_TypeDef *p_gpio_port;

    if(gpio_pin <= GPIO_PORT_A_7) {
        p_gpio_port = GPIOA;
    } else if(gpio_pin <= GPIO_PORT_B_7) {
        p_gpio_port = GPIOB;
    } else if(gpio_pin <= GPIO_PORT_C_7) {
        p_gpio_port = GPIOC;
    } else if(gpio_pin <= GPIO_PORT_D_7) {
        p_gpio_port = GPIOD;
    } else {
        return;
    }

    if(pin_val == 0) {
        // GPIO -> Low
        p_gpio_port->BCR = (1u << (gpio_pin & 0x07));
    } else {
        // GPIO -> High
        p_gpio_port->BSHR = (1u << (gpio_pin & 0x07));
    }
#endif
}