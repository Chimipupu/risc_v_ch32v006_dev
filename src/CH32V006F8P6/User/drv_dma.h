/**
 * @file drv_dma.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 DMAドライバ
 * @version 0.1
 * @date 2026-03-22
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef DRV_DMA_H
#define DRV_DMA_H


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ch32v00x.h>

// -----------------------------------------------------------
// [コンパイルスイッチ]

// -----------------------------------------------------------
#define DMA_CH_CNT            7

#define MODE_MEM2MEM       0x00
#define MODE_MEM2PERI      0x01

#define DATA_TYPE_BYTE     0x00 // 転送幅 ... 1バイト
#define DATA_TYPE_WORD     0x02 // 転送幅 ... 2バイト
#define DATA_TYPE_DWORD    0x04 // 転送幅 ... 4バイト

// -----------------------------------------------------------
typedef struct {
    uint8_t data_type;
    uint32_t size_byte;
    void *p_src;
    void *p_dst;
} drv_dma_config_t;

// -----------------------------------------------------------
void drv_dma_start(uint8_t ch);
bool drv_dma_tc_check(uint8_t ch);
bool drv_dma_init(uint8_t ch, uint8_t mode, drv_dma_config_t *p_config);

#endif // DRV_DMA_H