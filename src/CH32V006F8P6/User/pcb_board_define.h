/**
 * @file pcb_board.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 基板固有定義ヘッダ
 * @version 0.1
 * @date 2026-03-12
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef PCB_BOARD_H
#define PCB_BOARD_H

//**********************************************************************
// [コンパイルスイッチ]
#define PCB_ENV_BOARD_MK_0    0x00
#define PCB_ENV_BOARD_MK_1    0x01
#define PCB_TYPE    PCB_ENV_BOARD_MK_0
// #define PCB_TYPE    PCB_ENV_BOARD_MK_1

// ----------------------------------------------------------------------
// 試作初号基板 「ENV MK0」
#if (PCB_TYPE == PCB_ENV_BOARD_MK_0)
    #define PCB_NAME_STR    "ENV_MK0"

    #define DEBUG_UART_USE // UARTの使用有無
    #define DEBUG_I2C_USE  // I2Cの使用有無
    // #define EEPROM_USE

    #define I2C_RTC_DS3231                0
    #define I2C_RTC_RX8900                1
    #define I2C_RTC_DEVICE                I2C_RTC_DS3231
    // #define I2C_RTC_DEVICE                I2C_RTC_RX8900
    
    #define I2C_ENV_SENSOR_AHT20          0
    #define I2C_ENV_SENSOR_BMP280         1
    #define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_AHT20
    // #define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_BMP280
#endif // PCB_ENV_BOARD_MK_0
// ----------------------------------------------------------------------
// 試作壱号基板 「ENV MK1」
#if (PCB_TYPE == PCB_ENV_BOARD_MK_1)
    #define PCB_NAME_STR    "ENV_MK1"
    #define DEBUG_UART_USE  // UARTの使用有無
    #define DEBUG_I2C_USE   // I2Cの使用有無
    #define EEPROM_USE      // EEPROMの使用有無
#endif // PCB_ENV_BOARD_MK_1
//**********************************************************************

#endif // PCB_BOARD_H