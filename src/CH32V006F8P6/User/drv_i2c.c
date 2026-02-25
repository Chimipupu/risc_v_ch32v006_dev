/**
 * @file drv_i2c.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CH32V006 I2Cドライバ＆ラッパー＆API
 * @version 0.1
 * @date 2026-02-23
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#include "drv_i2c.h"

#if 0
#define I2C_SEND_BUF_SIZE    32
#define I2C_RECV_BUF_SIZE    32

volatile uint8_t g_i2c_send_buf[I2C_SEND_BUF_SIZE] = {0};
volatile uint8_t g_i2c_recv_buf[I2C_RECV_BUF_SIZE] = {0};
volatile uint8_t g_idx_i2c_send_buf = 0;
volatile uint8_t g_idx_i2c_recv_buf = 0;
#endif

//*********************************************************************]
// [API]

void drc_i2c_Init(uint32_t bound, uint16_t address)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitTSturcture  = {0};

    // ----------------------------------------------------------------------
    // [I2CのGPIO初期化]
    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOC | RCC_PB2Periph_AFIO, ENABLE );
    RCC_PB1PeriphClockCmd( RCC_PB1Periph_I2C1, ENABLE );

    // PC1 ... I2C SDA
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // オープンドレイン
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    // PC2 ... I2C SDA
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // オープンドレイン
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    I2C_InitTSturcture.I2C_ClockSpeed = bound;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );
    I2C_Cmd( I2C1, ENABLE );
    // ----------------------------------------------------------------------
}

/**
 * @brief I2C 送信API
 * @param slave_addr 通信相手のスレーブ 7bitアドレス(※左に1ビットシフトする前のアドレス)
 * @param p_send_data_buf 送信データバッファポインタ
 * @param data_len 送信したいデータバイト数
 * @return drv_i2c_ret 処理結果
 */
drv_i2c_ret drc_i2c_send(uint8_t slave_addr, uint8_t *p_send_data_buf, uint8_t data_len)
{
    uint8_t i;
    uint8_t *p_data;

#if 1
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
#else
    if(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET) {
        return I2C_RET_BUSY;
    }
#endif

    if((p_send_data_buf == NULL) || (data_len == 0)) {
        return I2C_RET_ERR;
    }

    p_data = p_send_data_buf;

    // Startビット
    I2C_GenerateSTART(I2C1, ENABLE);
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != READY);

    // 7Bitアドレス + WRITEビット送信
    I2C_Send7bitAddress(I2C1, slave_addr << 1, I2C_Direction_Transmitter);
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != READY);

    // データ送信
    for(i = 0; i < data_len; i++)
    {
        I2C_SendData(I2C1, *p_data);
        while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != READY);
        p_data++;
    }

    // Stopビット
    I2C_GenerateSTOP(I2C1, ENABLE);

    return I2C_RET_END;
}

/**
 * @brief I2C 受信API
 * @param slave_addr 通信相手のスレーブ 7bitアドレス(※左に1ビットシフトする前のアドレス)
 * @param p_recv_data_buf 受信データバッファポインタ
 * @param data_len 受信したいデータバイト数
 * @return drv_i2c_ret処理結果
 */
drv_i2c_ret drc_i2c_recv(uint8_t slave_addr, uint8_t *p_recv_data_buf, uint8_t data_len)
{
    uint8_t i;
    uint8_t *p_data;

#if 1
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
#else
    if(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET) {
        return I2C_RET_BUSY;
    }
#endif

    if((p_recv_data_buf == NULL) || (data_len == 0)) {
        return I2C_RET_ERR;
    }

    p_data = p_recv_data_buf;

    // Startビット
    I2C_GenerateSTART(I2C1, ENABLE);
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != READY);

    // 7Bitアドレス + Readビット送信
    I2C_Send7bitAddress(I2C1, slave_addr << 1, I2C_Direction_Receiver);
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != READY);

    for(i = 0; i < data_len; i++)
    {
        while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != READY);
        *p_data = I2C_ReceiveData(I2C1);
        p_data++;
        // if(i == (data_len - 1)) {
        //     I2C_AcknowledgeConfig(I2C1, DISABLE); // NACK
        // } else
        {
            I2C_AcknowledgeConfig(I2C1, ENABLE); // ACK
        }
    }

    // Stopビット
    I2C_GenerateSTOP(I2C1, ENABLE);

    return I2C_RET_END;
}

//*********************************************************************