/**
 * @file drv_i2c_eeprom_24c64.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief EEPROM 24C64 I2Cドライバ
 * @version 0.1
 * @date 2026-03-11
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_i2c_eeprom_24c64.h"
#include <stdint.h>
#include <string.h>

// -----------------------------------------------------------
// [Static関数]
void _eeprom_addr_to_buf(uint16_t addr, uint8_t *p_buf)
{
    if(p_buf == NULL) {
        return;
    }
    p_buf[0] = (uint8_t)((addr >> 8) & 0xFF); // アドレスの上位8bit
    p_buf[1] = (uint8_t)(addr & 0xFF);        // アドレスの下位8bit
}

// -----------------------------------------------------------
// [ドライバ]

drv_i2c_ret drv_eeprom_write_byte(uint16_t addr, uint8_t data)
{
    drv_i2c_ret ret = I2C_RET_END;
    uint8_t data_buf[3] = {0};

    // data[0]...アドレス上位8bit、data[1]...アドレス下位8bit
    _eeprom_addr_to_buf(addr, (uint8_t *)&data_buf[0]);
    // data[2]...書き込みデータ
    data_buf[2] = data;

    do {
        ret = drv_i2c_write(I2C_ADDR_EEPROM_24C64, (uint8_t *)&data_buf[0], 3, true);
    } while(ret != I2C_RET_END);

    return ret;
}

drv_i2c_ret drv_eeprom_read_byte(uint16_t addr, uint8_t *p_data)
{
    drv_i2c_ret ret = I2C_RET_END;
    uint8_t addr_buf[2] = {0};

    _eeprom_addr_to_buf(addr, (uint8_t *)&addr_buf[0]);

    do {
        ret = drv_i2c_write(I2C_ADDR_EEPROM_24C64, (uint8_t *)&addr_buf[0], 2, true);
    } while(ret != I2C_RET_END);

    do {
        ret = drv_i2c_read(I2C_ADDR_EEPROM_24C64, p_data, 1, true, true);
    } while(ret != I2C_RET_END);

    return ret;
}

drv_i2c_ret drv_eeprom_read_page(uint8_t read_page, uint8_t *p_page_data_buf)
{
    drv_i2c_ret ret = I2C_RET_END;
    uint8_t addr_buf[2] = {0};
    uint16_t page_top_addr;

    page_top_addr = read_page * EEPROM_24C64_PAGE_BYTE_SIZE;
    _eeprom_addr_to_buf(page_top_addr, (uint8_t *)&addr_buf[0]);

    do {
        ret = drv_i2c_write(I2C_ADDR_EEPROM_24C64, (uint8_t *)&addr_buf[0], 2, true);
    } while(ret != I2C_RET_END);

    do {
        ret = drv_i2c_read(I2C_ADDR_EEPROM_24C64, p_page_data_buf, EEPROM_24C64_PAGE_BYTE_SIZE, true, true);
    } while(ret != I2C_RET_END);

    return ret;
}

drv_i2c_ret drv_eeprom_write_page(uint8_t write_page, uint8_t *p_page_data_buf)
{
    drv_i2c_ret ret = I2C_RET_END;
    uint8_t data_buf[EEPROM_24C64_PAGE_BYTE_SIZE + 2] = {0};
    uint16_t page_top_addr;

    page_top_addr = write_page * EEPROM_24C64_PAGE_BYTE_SIZE;
    _eeprom_addr_to_buf(page_top_addr, (uint8_t *)&data_buf[0]);
    memcpy(&data_buf[2], p_page_data_buf, EEPROM_24C64_PAGE_BYTE_SIZE);

    do {
        ret = drv_i2c_write(I2C_ADDR_EEPROM_24C64,
                            (uint8_t *)&data_buf[0],
                            EEPROM_24C64_PAGE_BYTE_SIZE + 2,
                            true);
    } while(ret != I2C_RET_END);

    return ret;
}