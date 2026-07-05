/**
 * @file pcb_board.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 基板固有定義ヘッダ
 * @version 0.1
 * @date 2026-07-05
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef PCB_BOARD_H
#define PCB_BOARD_H

// -----------------------------------------------------------
// [コンパイルスイッチ]
#define I2C_RTC_NONE                  0xFF
#define I2C_RTC_DS3231                0
#define I2C_RTC_RX8900                1

#define I2C_ENV_SENSOR_NONE          0xFF
#define I2C_ENV_SENSOR_AHT20          0
#define I2C_ENV_SENSOR_BMP280         1

#define DEBUG_UART_USE // UARTの使用有無
// #define DEBUG_I2C_USE  // I2Cの使用有無
// #define USE_BUTTON  // 基板のボタン使用有無
// #define EEPROM_USE     // EEPROMの使用有無
#define USE_74HC595  // 74HC595の使用有無

#define I2C_RTC_DEVICE                I2C_ENV_SENSOR_NONE
// #define I2C_RTC_DEVICE                I2C_RTC_DS3231
// #define I2C_RTC_DEVICE                I2C_RTC_RX8900

#define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_NONE
// #define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_AHT20
// #define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_BMP280

// -----------------------------------------------------------
// [Define]
#define MCU_NAME               "CH32V006F8P6"
#define PCB_NAME               "DEV_PCB"
// #define PCB_NAME               "CH32V003F4P6-R0-1V1"

#define MCU_FLASH_SIZE         62
#define MCU_RAM_SIZE           8

// -----------------------------------------------------------

#endif // PCB_BOARD_H