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

// C Std Lib
#include "stdio.h"

// ADK
#include <ch32v00x.h>
#include "debug.h"

// My App
#include "drv_tim.h"
#include "drv_i2c.h"
#include "drv_uasrt.h"
#include "app_main.h"

// My Lib
#include "drv_rtc_rx8900.h"

int main(void)
{
#if 1
    volatile uint8_t dbg_rx_data[32] = {0};
#endif

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    // タイマー初期化
    drv_tim_init(65535, 48); // TIM1 @1us周期、カウントアップ @65.535ms

    // I2C初期化
    drc_i2c_Init(400000, I2C_ADDR_RTC_DS3231 << 1); // I2C マスター 400KHz

#if 1
    // DS3231の全アドレス0x00~0x12を一括読み出し
    while(1)
    {
        memset((uint8_t *)&dbg_rx_data[0], 0x00, 32);
        drc_i2c_send(I2C_ADDR_RTC_DS3231, 0x00, 1);
        drc_i2c_recv(I2C_ADDR_RTC_DS3231, (uint8_t *)&dbg_rx_data[0], 0x12);
        Delay_Ms(1000);
    }
#else
    // RX8900の全アドレス0x00~0x0Fを一括読み出し
    volatile const uint8_t dbg_tx_data[] = {RTC_RX8900_REG_CTRL, 0x00, 0x00, 0x00};
    drc_i2c_send((uint8_t *)&dbg_tx_data[0], sizeof(dbg_tx_data));
    drc_i2c_send(0x00, 1);
    drc_i2c_recv((uint8_t *)&dbg_rx_data[0], 0x0F);
#endif

#if 0

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
#endif

    return 0;
}