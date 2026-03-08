/**
 * @file app_io_reg.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief I/Oレジスタアプリ
 * @version 0.1
 * @date 2026-03-09
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#ifndef APP_IO_REG_H
#define APP_IO_REG_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define APP_IO_REG_NUM              128
#define APP_IO_REG_ADDR_WHO_I_AM    0x64

void app_io_reg_init(void);
uint8_t app_io_reg_read(uint8_t addr);
void app_io_reg_write(uint8_t addr, uint8_t val);

#endif // APP_IO_REG_H