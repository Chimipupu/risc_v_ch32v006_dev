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

// -----------------------------------------------------------
// [Define]

#define APP_IO_REG_NUM              128  // I/Oレジスタ数

// [レジスタアドレス]
#define APP_IO_REG_ADDR_WHO_I_AM    0x70 // Who_I_AMレジスタ
    #define WHO_I_AM_DEBUG          0x00
    #define WHO_I_AM_IOCPS          0x10
    #define WHO_I_AM_DATABUS        0x20
    #define WHO_I_AM_ROMRAMEMU      0x40
    #define WHO_I_AM_CPUEMU         0x80

#define APP_IO_REG_ADDR_DBG_0    0x78 // 開発時のDEBUG用レジスタ #0
#define APP_IO_REG_ADDR_DBG_1    0x79 // 開発時のDEBUG用レジスタ #1
#define APP_IO_REG_ADDR_DBG_2    0x7A // 開発時のDEBUG用レジスタ #2

// -----------------------------------------------------------
typedef struct {
    uint8_t addr;
    char *p_str;
} io_reg_str_t;

extern volatile const io_reg_str_t g_io_reg_str_tbl[];
extern volatile const uint8_t IO_REG_STR_TBL_CNT;

// -----------------------------------------------------------
void app_io_reg_init(void);
uint8_t app_io_reg_read(uint8_t addr);
void app_io_reg_write(uint8_t addr, uint8_t val);

#endif // APP_IO_REG_H