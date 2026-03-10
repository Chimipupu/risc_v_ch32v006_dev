/**
 * @file drv_i2c_eeprom_24c64.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief EEPROM 24C64 I2Cドライバ
 * @version 0.1
 * @date 2026-03-11
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_i2c_eeprom_24c64.h"

// -----------------------------------------------------------
// [ドライバ]
drv_i2c_ret drv_eeprom_write_byte(uint16_t addr, uint8_t *p_data_buf, uint16_t data_len)
{
    // TODO
}

drv_i2c_ret drv_eeprom_read_byte(uint16_t addr, uint8_t *p_data_buf, uint16_t data_len)
{
    // TODO
}

drv_i2c_ret drv_eeprom_write_page(uint8_t top_page, uint8_t *p_data_buf, uint8_t page_num)
{
    // TODO
}

drv_i2c_ret drv_eeprom_read_page(uint8_t top_page, uint8_t *p_data_buf, uint8_t page_num)
{
    // TODO
}