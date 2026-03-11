/**
 * @file drv_i2c_eeprom_24c64.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief EEPROM 24C64 I2Cドライバ
 * @version 0.1
 * @date 2026-03-11
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_I2C_EEPROM_24C64_H
#define DRV_I2C_EEPROM_24C64_H

#include "drv_i2c.h"

// -----------------------------------------------------------
// [Define]

#define I2C_ADDR_EEPROM_24C64         0x50 // EEPROM 24C64 I2Cスレーブアドレス
#define EEPROM_24C64_PAGE_SIZE_BYTE   32   // EEPROM 24C64 ページサイズ 32Byte
#define EEPROM_24C64_SIZE_BYTE        8192 // EEPROM 24C64 全容量 8192Byte
#define EEPROM_24C64_PAGE_NUM         (EEPROM_24C64_SIZE_BYTE / EEPROM_24C64_PAGE_SIZE_BYTE) // EEPROM 24C64 ページ数
#define EEPROM_WRITE_WAIT_TIME_MS     5    // EEPROM 24C64 書き込み完了までの待ち時間 5ms

// -----------------------------------------------------------
// [ドライバ]

drv_i2c_ret drv_eeprom_write_byte(uint16_t addr, uint8_t data);
drv_i2c_ret drv_eeprom_read_byte(uint16_t addr, uint8_t *p_data);
drv_i2c_ret drv_eeprom_write_page(uint8_t top_page, uint8_t *p_page_data_buf);
drv_i2c_ret drv_eeprom_read_page(uint8_t top_page, uint8_t *p_page_data_buf);
// -----------------------------------------------------------
#endif // DRV_I2C_EEPROM_24C64_H