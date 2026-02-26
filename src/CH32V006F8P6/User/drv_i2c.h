/**
 * @file drv_i2c.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 I2Cドライバ＆ラッパー＆API
 * @version 0.1
 * @date 2026-02-23
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#ifndef DRV_I2C_H
#define DRV_I2C_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

#define I2C_HOST_MODE     0
#define I2C_SLAVE_MODE    1

#define I2C_MODE   I2C_HOST_MODE
//#define I2C_MODE   I2C_SLAVE_MODE

#if (I2C_MODE == I2C_HOST_MODE)
#define I2C_CLOCK_100_KHZ    100000
#define I2C_CLOCK_400_KHZ    400000
#define I2C_CLOCK_1_MHZ      1000000

#define I2C_ADDR_E2P_AT24C32    0x57
#define I2C_ADDR_RTC_RX8900     0x32
#define I2C_ADDR_RTC_DS3231     0x68
#endif // I2C_HOST_MODE

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