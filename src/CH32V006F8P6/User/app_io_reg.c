/**
 * @file app_io_reg.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief I/Oレジスタアプリ
 * @version 0.1
 * @date 2026-03-09
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_io_reg.h"

// -----------------------------------------------------------
// [Private]

typedef struct {
    uint8_t read_bit;           // レジスタ属性: Readビット
    uint8_t write_bit;          // レジスタ属性: Writeビット
} app_io_reg_t;

// I/Oレジスタ属性テーブル
volatile const app_io_reg_t g_app_io_reg_attr_tbl[APP_IO_REG_NUM] = {
    // -----------------------------------------------------------------
    // [設定・状態関連レジスタ]
    {0xFF, 0xFF}, // Addr: 0, コントロール0レジスタ(属性: R/W @8bit)
    {0xFF, 0xFF}, // Addr: 1, コントロール1レジスタ(属性: R/W @8bit)
    {0xFF, 0xFF}, // Addr: 2, コントロール2レジスタ(属性: R/W @8bit)
    {0xFF, 0xFF}, // Addr: 3, ステータスレジスタ  (属性: R/W @8bit)
    {0x00, 0x00}, // Addr: 4 , (Rederved)
    {0x00, 0x00}, // Addr: 5 , (Rederved)
    {0x00, 0x00}, // Addr: 6 , (Rederved)
    {0x00, 0x00}, // Addr: 7 , (Rederved)
    {0x00, 0x00}, // Addr: 8 , (Rederved)
    {0x00, 0x00}, // Addr: 9 , (Rederved)
    {0x00, 0x00}, // Addr: 10, (Rederved)
    {0x00, 0x00}, // Addr: 11, (Rederved)
    {0x00, 0x00}, // Addr: 12, (Rederved)
    {0x00, 0x00}, // Addr: 13, (Rederved)
    {0x00, 0x00}, // Addr: 14, (Rederved)
    {0x00, 0x00}, // Addr: 15, (Rederved)
    // -----------------------------------------------------------------
    // [(Reserved)]
    {0xFF, 0xFF}, // Addr: 16, (Rederved)
    {0xFF, 0xFF}, // Addr: 17, (Rederved)
    {0xFF, 0xFF}, // Addr: 18, (Rederved)
    {0xFF, 0xFF}, // Addr: 19, (Rederved)
    {0x00, 0x00}, // Addr: 20, (Rederved)
    {0x00, 0x00}, // Addr: 21, (Rederved)
    {0x00, 0x00}, // Addr: 22, (Rederved)
    {0x00, 0x00}, // Addr: 23, (Rederved)
    {0x00, 0x00}, // Addr: 24, (Rederved)
    {0x00, 0x00}, // Addr: 25, (Rederved)
    {0x00, 0x00}, // Addr: 26, (Rederved)
    {0x00, 0x00}, // Addr: 27, (Rederved)
    {0x00, 0x00}, // Addr: 28, (Rederved)
    {0x00, 0x00}, // Addr: 29, (Rederved)
    {0x00, 0x00}, // Addr: 30, (Rederved)
    {0x00, 0x00}, // Addr: 31, (Rederved)
    // -----------------------------------------------------------------
    // [(Reserved)]
    {0x00, 0x00}, // Addr: 32, (Rederved)
    {0x00, 0x00}, // Addr: 33, (Rederved)
    {0x00, 0x00}, // Addr: 34, (Rederved)
    {0x00, 0x00}, // Addr: 35, (Rederved)
    {0x00, 0x00}, // Addr: 36, (Rederved)
    {0x00, 0x00}, // Addr: 37, (Rederved)
    {0x00, 0x00}, // Addr: 38, (Rederved)
    {0x00, 0x00}, // Addr: 39, (Rederved)
    {0x00, 0x00}, // Addr: 40, (Rederved)
    {0x00, 0x00}, // Addr: 41, (Rederved)
    {0x00, 0x00}, // Addr: 42, (Rederved)
    {0x00, 0x00}, // Addr: 43, (Rederved)
    {0x00, 0x00}, // Addr: 44, (Rederved)
    {0x00, 0x00}, // Addr: 45, (Rederved)
    {0x00, 0x00}, // Addr: 46, (Rederved)
    {0x00, 0x00}, // Addr: 47, (Rederved)
    {0x00, 0x00}, // Addr: 48, (Rederved)
    // -----------------------------------------------------------------
    // [(Reserved)]
    {0x00, 0x00}, // Addr: 49, (Rederved)
    {0x00, 0x00}, // Addr: 50, (Rederved)
    {0x00, 0x00}, // Addr: 51, (Rederved)
    {0x00, 0x00}, // Addr: 52, (Rederved)
    {0x00, 0x00}, // Addr: 53, (Rederved)
    {0x00, 0x00}, // Addr: 54, (Rederved)
    {0x00, 0x00}, // Addr: 55, (Rederved)
    {0x00, 0x00}, // Addr: 56, (Rederved)
    {0x00, 0x00}, // Addr: 57, (Rederved)
    {0x00, 0x00}, // Addr: 58, (Rederved)
    {0x00, 0x00}, // Addr: 59, (Rederved)
    {0x00, 0x00}, // Addr: 60, (Rederved)
    {0x00, 0x00}, // Addr: 61, (Rederved)
    {0x00, 0x00}, // Addr: 62, (Rederved)
    {0x00, 0x00}, // Addr: 63, (Rederved)
    {0x00, 0x00}, // Addr: 64, (Rederved)
    // -----------------------------------------------------------------
    // [(Reserved)]
    {0x00, 0x00}, // Addr: 65, (Rederved)
    {0x00, 0x00}, // Addr: 66, (Rederved)
    {0x00, 0x00}, // Addr: 67, (Rederved)
    {0x00, 0x00}, // Addr: 68, (Rederved)
    {0x00, 0x00}, // Addr: 69, (Rederved)
    {0x00, 0x00}, // Addr: 70, (Rederved)
    {0x00, 0x00}, // Addr: 71, (Rederved)
    {0x00, 0x00}, // Addr: 72, (Rederved)
    {0x00, 0x00}, // Addr: 73, (Rederved)
    {0x00, 0x00}, // Addr: 74, (Rederved)
    {0x00, 0x00}, // Addr: 75, (Rederved)
    {0x00, 0x00}, // Addr: 76, (Rederved)
    {0x00, 0x00}, // Addr: 77, (Rederved)
    {0x00, 0x00}, // Addr: 78, (Rederved)
    {0x00, 0x00}, // Addr: 79, (Rederved)
    {0x00, 0x00}, // Addr: 80, (Rederved)
    // -----------------------------------------------------------------
    // [(Reserved)]
    {0x00, 0x00}, // Addr: 81, (Rederved)
    {0x00, 0x00}, // Addr: 82, (Rederved)
    {0x00, 0x00}, // Addr: 83, (Rederved)
    {0x00, 0x00}, // Addr: 84, (Rederved)
    {0x00, 0x00}, // Addr: 85, (Rederved)
    {0x00, 0x00}, // Addr: 86, (Rederved)
    {0x00, 0x00}, // Addr: 87, (Rederved)
    {0x00, 0x00}, // Addr: 88, (Rederved)
    {0x00, 0x00}, // Addr: 89, (Rederved)
    {0x00, 0x00}, // Addr: 90, (Rederved)
    {0x00, 0x00}, // Addr: 91, (Rederved)
    {0x00, 0x00}, // Addr: 92, (Rederved)
    {0x00, 0x00}, // Addr: 93, (Rederved)
    {0x00, 0x00}, // Addr: 94, (Rederved)
    {0x00, 0x00}, // Addr: 95, (Rederved)
    {0x00, 0x00}, // Addr: 96, (Rederved)
    // -----------------------------------------------------------------
    // [(Reserved)]
    {0x00, 0x00}, // Addr: 97　, (Rederved)
    {0x00, 0x00}, // Addr: 98　, (Rederved)
    {0x00, 0x00}, // Addr: 99　, (Rederved)
    {0x00, 0x00}, // Addr: 100, (Rederved)
    {0x00, 0x00}, // Addr: 101, (Rederved)
    {0x00, 0x00}, // Addr: 102, (Rederved)
    {0x00, 0x00}, // Addr: 103, (Rederved)
    {0x00, 0x00}, // Addr: 104, (Rederved)
    {0x00, 0x00}, // Addr: 105, (Rederved)
    {0x00, 0x00}, // Addr: 106, (Rederved)
    {0x00, 0x00}, // Addr: 107, (Rederved)
    {0x00, 0x00}, // Addr: 108, (Rederved)
    {0x00, 0x00}, // Addr: 109, (Rederved)
    {0x00, 0x00}, // Addr: 110, (Rederved)
    {0x00, 0x00}, // Addr: 111, (Rederved)
    // -----------------------------------------------------------------
    // [デバッグ関連レジスタ]
    {0xFF, 0x00}, // Addr: 112, WHO_I_AMレジスタ(属性: RO @8bit)
    {0x00, 0x00}, // Addr: 113, (Rederved)
    {0x00, 0x00}, // Addr: 114, (Rederved)
    {0x00, 0x00}, // Addr: 115, (Rederved)
    {0x00, 0x00}, // Addr: 116, (Rederved)
    {0x00, 0x00}, // Addr: 117, (Rederved)
    {0x00, 0x00}, // Addr: 118, (Rederved)
    {0x00, 0x00}, // Addr: 119, (Rederved)
    {0xFF, 0xFF}, // Addr: 120, 開発時のDEBUG用レジスタ #0
    {0xFF, 0xFF}, // Addr: 121, 開発時のDEBUG用レジスタ #1
    {0xFF, 0xFF}, // Addr: 122, 開発時のDEBUG用レジスタ #2
    {0x00, 0x00}, // Addr: 123, (Rederved)
    {0x00, 0x00}, // Addr: 124, (Rederved)
    {0x00, 0x00}, // Addr: 125, (Rederved)
    {0x00, 0x00}, // Addr: 126, (Rederved)
    {0x00, 0x00}, // Addr: 127, (Rederved)
};

// I/Oレジスタデータテーブル
volatile const io_reg_data_t g_io_reg_data_tbl[] = {
    {APP_IO_REG_ADDR_WHO_I_AM, "WIA"  , WHO_I_AM_IOCPS},
    {APP_IO_REG_ADDR_DBG_0,    "DBG_0", 0x00},
    {APP_IO_REG_ADDR_DBG_1,    "DBG_1", 0x00},
    {APP_IO_REG_ADDR_DBG_2,    "DBG_2", 0x00},
};
volatile const uint8_t IO_REG_STR_TBL_CNT = sizeof(g_io_reg_data_tbl) / sizeof(g_io_reg_data_tbl[0]);

static uint8_t s_app_io_reg[APP_IO_REG_NUM] = {0};

// -----------------------------------------------------------
// [Static関数]

// -----------------------------------------------------------
// [API]

void app_io_reg_init(void)
{
    uint8_t i;

    memset(&s_app_io_reg[0], 0x00, sizeof(s_app_io_reg));

    for(i = 0; i < IO_REG_STR_TBL_CNT; i++)
    {
        s_app_io_reg[g_io_reg_data_tbl[i].addr] = g_io_reg_data_tbl[i].init_val;
    }
}

uint8_t app_io_reg_read(uint8_t addr)
{
    uint8_t reg;

    if(addr < APP_IO_REG_NUM) {
        reg = s_app_io_reg[addr] & g_app_io_reg_attr_tbl[addr].read_bit;
    } else {
        reg = 0x00;
    }

    return reg;
}

void app_io_reg_write(uint8_t addr, uint8_t val)
{
    if(addr < APP_IO_REG_NUM) {
        s_app_io_reg[addr] = val & g_app_io_reg_attr_tbl[addr].write_bit;
    }
}