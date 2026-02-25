/**
 * @file main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V003 メイン
 * @version 0.1
 * @date 2025-08-06
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "stdio.h"
#include <ch32v00x.h>

#include "debug.h"
#include "drv_tim.h"
#include "drv_i2c.h"
#include "drv_uasrt.h"
#include "app_main.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    // タイマー初期化
    drv_tim_init(65535, 48); // TIM1 @1us周期、カウントアップ @65.535ms

    // I2C初期化
    drc_i2c_Init(400000, 0x32); // I2C マスター 400KHz

    // UART初期化
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#endif
#ifdef DEBUG_UART_USE
    // USRAT初期化 115200 8N1(TX=PD5ピン, RX=PD6ピン)
    hw_usart_init();
#endif //DEBUG_UART_USE
#if 0
    printf("[DEBUG] CH32V006F8P6 Develop\r\n");
    printf("SystemClk:%d\r\n",SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
#endif

    // アプリメイン初期化
    app_main_init();

    while(1)
    {
        // アプリメイン
        app_main();
    }

    return 0;
}