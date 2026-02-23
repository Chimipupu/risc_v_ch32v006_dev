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

#define HOST_MODE   0
#define SLAVE_MODE   1

#define I2C_MODE   HOST_MODE
//#define I2C_MODE   SLAVE_MODE

/*
 * 0 - start
 * 1 - After sending the start signal
 * 2 - After sending the write address
 * 3 - End of send data
 * 4 - After sending the repeat start signal
 * 5 - After sending the read address
 * 0xff - end
 * */
typedef enum {
    I2C_STATE_START = 0x00,
#if (I2C_MODE == HOST_MODE)
    I2C_STATE_AFTER_START,
    I2C_STATE_AFTER_WRITE_ADDR,
    I2C_STATE_END_OF_SEND_DATA,
    I2C_STATE_AFTER_REPEAT_START,
    I2C_STATE_AFTER_READ_ADDR,
#endif
    I2C_STATE_END = 0xFF
} e_i2c_state;

#endif // DRV_I2C_H