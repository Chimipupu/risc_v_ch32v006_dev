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

extern volatile uint8_t g_req_i2c_send_data_len;
extern volatile uint8_t g_req_i2c_recv_data_len;

#if (I2C_MODE == HOST_MODE)
volatile e_i2c_state g_i2c_master_sate = I2C_STATE_START;
volatile uint16_t g_i2c_master_recv_len = 0;
volatile uint16_t g_i2c_master_send_len = 0;
#else
volatile e_i2c_state g_i2c_slave_state = 0;
volatile uint16_t g_i2c_slave_recv_len = 0;
volatile uint16_t g_i2c_slave_send_len = 0;
#endif

uint8_t g_i2c_send_buf[I2C_SEND_BUF_SIZE] = {0};
uint8_t g_i2c_recv_buf[I2C_RECV_BUF_SIZE] = {0};


void I2C1_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_ER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void I2C1_EV_IRQHandler(void )
{
#if (I2C_MODE == HOST_MODE)
    // -------------------------------------------------------------
    // SB (Start Bit): START発行完了
    // -------------------------------------------------------------
    if( I2C_GetITStatus( I2C1, I2C_IT_SB ) != RESET ) {
        if(g_i2c_master_sate == I2C_STATE_START) {
            g_i2c_master_sate = I2C_STATE_AFTER_START;
            I2C_Send7bitAddress( I2C1, RTC_RX8900_I2C_SLAVE_ADDR, I2C_Direction_Transmitter );
        } else {
            g_i2c_master_sate = I2C_STATE_AFTER_REPEAT_START;
            I2C_Send7bitAddress( I2C1, RTC_RX8900_I2C_SLAVE_ADDR, I2C_Direction_Receiver );
        }
    }
    // -------------------------------------------------------------
    // ADDR (Address Sent): アドレス一致・送信完了
    // -------------------------------------------------------------
    else if( I2C_GetITStatus( I2C1, I2C_IT_ADDR ) != RESET ) {
        if(g_i2c_master_sate == I2C_STATE_AFTER_START) {
            g_i2c_master_sate = I2C_STATE_AFTER_WRITE_ADDR;
            I2C_ITConfig( I2C1, I2C_IT_BUF, ENABLE );
        }
        else if(g_i2c_master_sate == I2C_STATE_AFTER_REPEAT_START) {
            g_i2c_master_sate = I2C_STATE_AFTER_READ_ADDR;
            I2C_ITConfig( I2C1, I2C_IT_BUF, ENABLE );
        }
        ((void)I2C_ReadRegister(I2C1, I2C_Register_STAR2));
    }
    // -------------------------------------------------------------
    // TXE (Transmit Data Reg Empty): 送信データレジスタ空
    // -------------------------------------------------------------
    else if( I2C_GetITStatus( I2C1, I2C_IT_TXE ) != RESET ) {
        if(g_i2c_master_sate == I2C_STATE_AFTER_WRITE_ADDR) {
            if(g_i2c_master_send_len < g_req_i2c_send_data_len) {
                I2C_SendData( I2C1, g_i2c_send_buf[g_i2c_master_send_len] );
                g_i2c_master_send_len++;
            } else {
                g_i2c_master_sate = I2C_STATE_END_OF_SEND_DATA;
                I2C_ITConfig( I2C1, I2C_IT_BUF, DISABLE );
                I2C_GenerateSTART(I2C1, ENABLE);
            }
        }
    }
    // -------------------------------------------------------------
    // RXNE (Receive Data Reg Not Empty): 受信データレジスタにデータあり
    // -------------------------------------------------------------
    else if( I2C_GetITStatus( I2C1, I2C_IT_RXNE ) != RESET ) {
        if(g_i2c_master_sate == I2C_STATE_AFTER_READ_ADDR) {
            if(g_i2c_master_recv_len < g_req_i2c_recv_data_len) {
                if(g_i2c_master_recv_len == (g_req_i2c_recv_data_len - 1)) {
                    I2C_AcknowledgeConfig(I2C1, DISABLE);
                    I2C_GenerateSTOP( I2C1, ENABLE );
                }
                g_i2c_recv_buf[g_i2c_master_recv_len] = I2C_ReceiveData(I2C1);
                g_i2c_master_recv_len++;

                if(g_i2c_master_recv_len == g_req_i2c_recv_data_len) {
                    g_i2c_master_sate = I2C_STATE_END;
                    I2C_ITConfig( I2C1, I2C_IT_BUF, DISABLE );
                }
            }
        }
    }
#endif
}

void I2C1_ER_IRQHandler(void)
{
// I2Cエラー割り込み @I2Cホスト
#if (I2C_MODE == HOST_MODE)
    // TODO:
// I2Cエラー割り込み @I2Cホスト
#elif (I2C_MODE == SLAVE_MODE)
    if( I2C_GetITStatus( I2C1, I2C_IT_AF ) )
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
        g_i2c_slave_state = 0xff;
    }else{
        //err
    }
#endif
}
//*********************************************************************
void drc_i2c_Init(u32 bound, u16 address)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitTSturcture  = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    // ----------------------------------------------------------------------
    // [I2CのGPIO初期化]
    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOC | RCC_PB2Periph_AFIO, ENABLE );
    RCC_PB1PeriphClockCmd( RCC_PB1Periph_I2C1, ENABLE );

    // PC1 ... I2C SDA
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    // PC2 ... I2C SDA
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
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
    // [I2Cの割り込み初期化]
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    // NOTE: 初期段階では BUF 割り込み (TXE/RXNE) は無効にしておく
    // NOTE: I2C_IT_ADDR 割り込みなどで適宜有効化する
    I2C_ITConfig( I2C1, I2C_IT_BUF, DISABLE );
    I2C_ITConfig( I2C1, I2C_IT_EVT, ENABLE );
    I2C_ITConfig( I2C1, I2C_IT_ERR, ENABLE );
    // ----------------------------------------------------------------------
    // バッファ初期化
    memset(g_i2c_send_buf, 0x00, I2C_SEND_BUF_SIZE);
    memset(g_i2c_recv_buf, 0x00, I2C_RECV_BUF_SIZE);
    // ----------------------------------------------------------------------
}

//*********************************************************************