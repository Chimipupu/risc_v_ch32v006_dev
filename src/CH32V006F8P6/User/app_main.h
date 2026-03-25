/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2026-02-25
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
// #include <math.h>

// ----------------------------------------------------------------------
// [コンパイルスイッチ]
#define DEBUG_APP
// #define USE_DEBUG_PRINTF
#define DBG_COM_USE

// ----------------------------------------------------------------------
// [マクロ]
#define APP_PROC_EXEC     0x00 // アプリ実行継続
#define APP_PROC_END      0x01 // アプリ実行終了
#define APP_PROC_ERROR    0xFF // アプリ実行エラー

// レジスタを8/16/32bitでR/Wするマクロ
#define REG_READ_BYTE(base, offset)           (*(volatile uint8_t  *)((base) + (offset)))
#define REG_READ_WORD(base, offset)           (*(volatile uint16_t *)((base) + (offset)))
#define REG_READ_DWORD(base, offset)          (*(volatile uint32_t *)((base) + (offset)))
#define REG_WRITE_BYTE(base, offset, val)     (*(volatile uint8_t  *)((base) + (offset)) = (val))
#define REG_WRITE_WORD(base, offset, val)     (*(volatile uint16_t *)((base) + (offset)) = (val))
#define REG_WRITE_DWORD(base, offset, val)    (*(volatile uint32_t *)((base) + (offset)) = (val))

// レジスタビット操作マクロ
#define REG_BIT_SET(reg, bit)                 ((reg) |=  (1UL << (bit))) // レジスタのビットをセット
#define REG_BIT_CLR(reg, bit)                 ((reg) &= ~(1UL << (bit))) // レジスタのビットをクリア
#define REG_BIT_TGL(reg, bit)                 ((reg) ^=  (1UL << (bit))) // レジスタのビットをトグル
#define REG_BIT_CHK(reg, bit)                 ((reg) &   (1UL << (bit))) // レジスタのビットチェック

// ----------------------------------------------------------------------
// [define]

// CH32V006 レジスタアドレス
#define REG_ADDR_R16_ESIG_FLACAP              0x1FFFF7E0
#define REG_ADDR_R32_ESIG_UNIID1              0x1FFFF7E8
#define REG_ADDR_R32_ESIG_UNIID2              0x1FFFF7EC
#define REG_ADDR_R32_ESIG_UNIID3              0x1FFFF7F0

// ----------------------------------------------------------------------

// NOP
__attribute__( ( always_inline ) ) static inline void NOP(void)
{
    __asm__ __volatile__("nop");
}

// 割り込み禁止
__attribute__( ( always_inline ) ) static inline void _DI(void)
{
    __asm__ __volatile__("csrci mstatus, 0x8");
}

// 割り込み許可
__attribute__( ( always_inline ) ) static inline void _EI(void)
{
    __asm__ __volatile__("csrsi mstatus, 0x8");
}

// ----------------------------------------------------------------------
void app_util_chip_uid_read(uint32_t *p_buf);
void app_main_init(void);
void app_main(void);

#endif // APP_MAIN_H