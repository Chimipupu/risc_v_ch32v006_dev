/**
 * @file drv_i2c.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 I2Cドライバ
 * @version 0.1
 * @date 2026-02-23
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_I2C_H
#define DRV_I2C_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

// -----------------------------------------------------------
// [コンパイルスイッチ]
#define I2C_HOST_MODE     0
#define I2C_SLAVE_MODE    1

#define I2C_MODE   I2C_HOST_MODE
//#define I2C_MODE   I2C_SLAVE_MODE

// I2Cクロック
#define I2C_CLOCK_100_KHZ             100000  // I2C SCLクロック = 100KHz
#define I2C_CLOCK_400_KHZ             400000  // I2C SCLクロック = 400KHz
#define I2C_CLOCK_1_MHZ               1000000 // I2C SCLクロック = 1MHz

#define I2C_RTC_DS3231                0
#define I2C_RTC_RX8900                1
#define I2C_RTC_DEVICE                I2C_RTC_DS3231
// #define I2C_RTC_DEVICE                I2C_RTC_RX8900

#define I2C_ENV_SENSOR_AHT20          0
#define I2C_ENV_SENSOR_BMP280         1
#define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_AHT20
// #define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_BMP280
// -----------------------------------------------------------
// [Define]

// [I2Cスレーブアドレス]
#define I2C_ADDR_RTC_RX8900           0x32 // RTC RX8900 I2Cスレーブアドレス
#define I2C_ADDR_SENSOR_AHT20         0x38 // AHT20 湿温度センサ I2Cスレーブアドレス
#define I2C_ADDR_E2P_AT24C32          0x57 // EEPROM AT24C32 I2Cスレーブアドレス
#define I2C_ADDR_RTC_DS3231           0x68 // RTC DS3231 I2Cスレーブアドレス
// #define I2C_ADDR_SENSOR_BMP280        0x76 // BMP280 温度+気圧センサ I2Cスレーブアドレス
#define I2C_ADDR_SENSOR_BMP280        0x77 // BMP280 温度+気圧センサ I2Cスレーブアドレス

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280)
// [BMP280 レジスタアドレス]
#define BMP280_REG_ADDR_TEMP_XLSB     0xFC
#define BMP280_REG_ADDR_TEMP_LSB      0xFB
#define BMP280_REG_ADDR_TEMP_MSB      0xFA
#define BMP280_REG_ADDR_PRESS_XLSB    0xF9
#define BMP280_REG_ADDR_PRESS_LSB     0xF8
#define BMP280_REG_ADDR_PRESS_MSB     0xF7
#define BMP280_REG_ADDR_CONFIG        0xF5
#define BMP280_REG_ADDR_CTRL_MEAS     0xF4
#define BMP280_REG_ADDR_STATUS        0xF3
#define BMP280_REG_ADDR_RESET         0xE0
#define BMP280_REG_ADDR_ID            0xD0

// [BMP280 IDレジスタ 期待値]
#define BMP280_ID_REG_EXP_VAL         0x58

// [BMP280 RESETレジスタ 書き込み値]
#define BMP280_RESET_REG_EXP_VAL      0xB6

#endif //I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280

// -----------------------------------------------------------
typedef enum {
    I2C_RET_BUSY = 0,   // Busy
    I2C_RET_EXEC,       // 処理中
    I2C_RET_END,        // 処理完了
    I2C_RET_ERR = 0xFF, // エラー
} drv_i2c_ret;

//*********************************************************************]
// [API]

#if (I2C_MODE == I2C_SLAVE_MODE)
void drc_i2c_Init(uint32_t bound, uint16_t address);
#else
void drc_i2c_Init(uint32_t bound);
#endif
// ----------------------------------------------------------------------
/**
 * @brief I2C 送信API
 * @param slave_addr 通信相手のスレーブ 7bitアドレス(※左に1ビットシフトする前のアドレス)
 * @param p_send_data_buf 送信データバッファポインタ
 * @param data_len 送信したいデータバイト数
 * @return drv_i2c_ret 処理結果
 */
drv_i2c_ret drc_i2c_send(uint8_t slave_addr, uint8_t *p_send_data_buf, uint8_t data_len);
// ----------------------------------------------------------------------
/**
 * @brief I2C 受信API
 * @param slave_addr 通信相手のスレーブ 7bitアドレス(※左に1ビットシフトする前のアドレス)
 * @param p_recv_data_buf 受信データバッファポインタ
 * @param data_len 受信したいデータバイト数
 * @param nack_opt 最後のデータ - 1のときにNACKを出すか否か
 * @return drv_i2c_ret処理結果
 */
drv_i2c_ret drc_i2c_recv(uint8_t slave_addr, uint8_t *p_recv_data_buf, uint8_t data_len, bool nack_opt);
//*********************************************************************

#endif // DRV_I2C_H