/**
 * @file main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V003 メイン
 * @version 0.1
 * @date 2025-08-06
 * 
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 * 
 */

// C Std Lib
#include "stdio.h"

// ADK
#include <ch32v00x.h>
#include "debug.h"

// My App
#include "app_main.h"
#include "drv_tim.h"
#include "drv_i2c.h"
#include "drv_uasrt.h"

// My Lib
#include "drv_rtc_rx8900.h"

// -----------------------------------------------------------
// [Private]
static void hw_clock_init(void);
static void hw_timer_init(void);
static void hw_uart_init(void);
static void hw_i2c_init(void);

// -----------------------------------------------------------
// [Static関数]

static void hw_clock_init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
}

static void hw_timer_init(void)
{
    // SysTickタイマー初期化
    NVIC_EnableIRQ(SysTick_IRQn);                 // SysTick割り込み有効化
    SysTick->SR &= ~(1 << 0);                     // SysTick割り込みフラグクリア
    SysTick->CMP = (SystemCoreClock - 1) / 1000;  // SysTick割り込み = 1ms周期
    SysTick->CNT = 0;                             // SysTickカウント値をクリア
    SysTick->CTLR = 0xF;

    // TIM1 (16bit 高機能タイマー)初期化
#if 0
    // TIM1 @1ms周期、カウントアップ @65.535秒
    // 1ms周期 = (48MHz / div) / psc = (48MHz / 1) / 48000 = 1ms
    drv_tim_init(65535, 48000, TIM_CKD_DIV1);
#else
    // TIM1 @1us周期、カウントアップ @65.535ms
    drv_tim_init(65535, 48, TIM_CKD_DIV1);
#endif
}

static void hw_uart_init(void)
{
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#endif
#ifdef DEBUG_UART_USE
    // USRAT初期化 115200 8N1(TX=PD5ピン, RX=PD6ピン)
    drv_uart_init();
#endif //DEBUG_UART_USE

#if (SDI_PRINT == SDI_PR_OPEN) || defined(DEBUG_UART_USE)
    printf("[DEBUG] CH32V006F8P6 Develop\r\n");
    printf("SystemClk: %d MHz\r\n",SystemCoreClock / 1000000);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID() );
#endif
}

static void hw_i2c_init(void)
{
#ifdef DEBUG_I2C_USE
    drc_i2c_Init(I2C_CLOCK_400_KHZ); // I2C マスター 400KHz
#endif // DEBUG_I2C_USE
}

// -----------------------------------------------------------
// [メイン関連関数]

/**
 * @brief メイン関数
 * @return int (未使用)
 */
int main(void)
{
    // [ドライバ関連]
    hw_clock_init(); // クロック初期化
    hw_timer_init(); // タイマー初期化
    hw_i2c_init();   // I2C初期化
    hw_uart_init();  // UART初期化

    // [アプリ関連]
    app_main_init(); // アプリメイン初期化

    while(1)
    {
        app_main(); // アプリメイン
    }

    return 0;
}