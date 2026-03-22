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
#define MODE_MEM2MEM       0x00
#define MODE_MEM2PERI      0x01

#define DATA_TYPE_BYTE     0x00
#define DATA_TYPE_WORD     0x01
#define DATA_TYPE_DWORD    0x02

// -----------------------------------------------------------
typedef struct {
    uint8_t mode;
    uint32_t size;
    uint8_t data_type;
    void *p_src;
    void *p_dst;
} drv_dma_config_t;

// -----------------------------------------------------------
void drv_dma_init(uint8_t ch, drv_dma_config_t *p_config);

#endif // DRV_DMA_H