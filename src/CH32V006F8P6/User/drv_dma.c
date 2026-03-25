/**
 * @file drv_dma.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 DMAドライバ
 * @version 0.1
 * @date 2026-03-22
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_dma.h"

// -----------------------------------------------------------

typedef struct {
    DMA_Channel_TypeDef *p_dma_ch;
    uint32_t tc_flg;
} dma_config_data_t;

const dma_config_data_t g_dma_ch_config_tbl[DMA_CH_CNT] = {
    {DMA1_Channel1, DMA1_FLAG_TC1},
    {DMA1_Channel2, DMA1_FLAG_TC2},
    {DMA1_Channel3, DMA1_FLAG_TC3},
    {DMA1_Channel4, DMA1_FLAG_TC4},
    {DMA1_Channel5, DMA1_FLAG_TC5},
    {DMA1_Channel6, DMA1_FLAG_TC6},
    {DMA1_Channel7, DMA1_FLAG_TC7},
};

static drv_dma_config_t s_dma_config_tbl[DMA_CH_CNT];

void _dma_init(uint8_t ch, uint8_t mode);
// -----------------------------------------------------------
// [Static関数]

void _dma_init(uint8_t ch, uint8_t mode)
{
    DMA_InitTypeDef DMA_InitStructure = {0};
    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);

    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = s_dma_config_tbl[ch].size_byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

    switch (s_dma_config_tbl[ch].data_type)
    {
        // 転送幅 ... 2バイト
        case DATA_TYPE_WORD:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)((uint16_t *)s_dma_config_tbl[ch].p_src);
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)((uint16_t *)s_dma_config_tbl[ch].p_dst);
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
            break;

        // 転送幅 ... 4バイト
        case DATA_TYPE_DWORD:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)((uint32_t *)s_dma_config_tbl[ch].p_src);
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)((uint32_t *)s_dma_config_tbl[ch].p_dst);
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
            break;

        // 転送幅 ... 1バイト
        case DATA_TYPE_BYTE:
        default:
            DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)((uint8_t *)s_dma_config_tbl[ch].p_src);
            DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)((uint8_t *)s_dma_config_tbl[ch].p_dst);
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
            break;
    }

    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;

    if(mode == MODE_MEM2MEM) {
        DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
    } else {
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    }

    DMA_Init(g_dma_ch_config_tbl[ch].p_dma_ch, &DMA_InitStructure);
    DMA_ClearFlag(g_dma_ch_config_tbl[ch].tc_flg);
}
// -----------------------------------------------------------
// [ドライバ]

void drv_dma_start(uint8_t ch)
{
    if(ch < DMA_CH_CNT) {
        DMA_Cmd(g_dma_ch_config_tbl[ch].p_dma_ch, ENABLE);
    }
}

bool drv_dma_tc_check(uint8_t ch)
{
    if(ch < DMA_CH_CNT) {
        if(DMA_GetFlagStatus(g_dma_ch_config_tbl[ch].tc_flg) == SET) {
            return true;
        } else {
            return false;
        }
    }
}

bool drv_dma_init(uint8_t ch, uint8_t mode, drv_dma_config_t *p_config)
{
    bool ret = false;

    if((ch < DMA_CH_CNT) && (mode <= MODE_MEM2PERI) && (p_config->data_type <= DATA_TYPE_DWORD)) {
        s_dma_config_tbl[ch].data_type = p_config->data_type;
        s_dma_config_tbl[ch].size_byte = p_config->size_byte;
        s_dma_config_tbl[ch].p_src = p_config->p_src;
        s_dma_config_tbl[ch].p_dst = p_config->p_dst;

        _dma_init(ch, mode);
    }

    return ret;
}