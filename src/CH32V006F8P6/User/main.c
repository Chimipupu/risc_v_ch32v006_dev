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
#include "dbg_com.h"
#include "drv_uasrt.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    // USRAT初期化 115200 8N1(TX=PD5ピン, RX=PD6ピン)
    hw_usart_init();

    printf("[DEBUG] CH32V006F8P6 Develop\r\n");
    printf("SystemClk:%d\r\n",SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );


    // デバッグモニタ 初期化
    dbg_com_init();

    while(1)
    {
        // デバッグモニタ メイン
        dbg_com_main();
    }

    return 0;
}