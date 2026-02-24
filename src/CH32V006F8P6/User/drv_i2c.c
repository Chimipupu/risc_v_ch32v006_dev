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

#if (I2C_MODE == HOST_MODE)
volatile e_i2c_state g_i2c_master_sate = I2C_STATE_START;
volatile uint16_t g_i2c_master_recv_len = 0; // ドライバで送信したデータバイト数
volatile uint16_t g_i2c_master_send_len = 0; // ドライバで受信したデータバイト数
#else
volatile e_i2c_state g_i2c_slave_state = 0;
volatile uint16_t g_i2c_slave_recv_len = 0; // ドライバで送信したデータバイト数
volatile uint16_t g_i2c_slave_send_len = 0; // ドライバで受信したデータバイト数
#endif


volatile uint8_t g_i2c_send_buf[I2C_SEND_BUF_SIZE] = {0};
volatile uint8_t g_i2c_recv_buf[I2C_RECV_BUF_SIZE] = {0};
volatile uint8_t g_idx_i2c_send_buf = 0;
volatile uint8_t g_idx_i2c_recv_buf = 0;
static bool s_is_send_req = false;
static bool s_is_recv_req = false;
static bool s_is_send_done = false;
static bool s_is_recv_done = false;
volatile uint8_t g_req_i2c_send_data_len; // 呼び元の送信要求データバイト数
volatile uint8_t g_req_i2c_recv_data_len; // 呼び元の受信要求データバイト数

void I2C1_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_ER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

//*********************************************************************
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
    // TXE (Transmit Data Reg Empty): 送信データレジスタが空でっせ
    // -------------------------------------------------------------
    else if( I2C_GetITStatus( I2C1, I2C_IT_TXE ) != RESET ) {
        if(g_i2c_master_sate == I2C_STATE_AFTER_WRITE_ADDR) {
            if(g_i2c_master_send_len < g_req_i2c_send_data_len) {
                I2C_SendData( I2C1, g_i2c_send_buf[g_i2c_master_send_len] );
                g_i2c_master_send_len++;
            } else {
                s_is_send_done = true;
                g_i2c_master_sate = I2C_STATE_END_OF_SEND_DATA;
                I2C_ITConfig( I2C1, I2C_IT_BUF, DISABLE );
                I2C_GenerateSTART(I2C1, ENABLE);
            }
        }
    }
    // -------------------------------------------------------------
    // RXNE (Receive Data Reg Not Empty): 受信データレジスタになんかデータあるで
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
                    s_is_recv_done = true;
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
//*********************************************************************]
// [API]

void drc_i2c_Init(uint32_t bound, uint16_t address)
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
}

/**
 * @brief I2C 送信API
 * @param p_send_data_buf 送信データバッファポインタ
 * @param data_len 送信したいデータバイト数
 * @return drv_i2c_ret 処理結果
 */
drv_i2c_ret drc_i2c_send(uint8_t *p_send_data_buf, uint8_t data_len)
{
    if((s_is_send_req == true) || (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)) {
        return I2C_RET_BUSY;
    }

    if((p_send_data_buf == NULL) || (data_len == 0)) {
        return I2C_RET_ERR;
    }

    if(s_is_send_done != true) {
        memset((void *)&g_i2c_send_buf[0], 0x00, I2C_SEND_BUF_SIZE);
        memcpy((void *)&g_i2c_send_buf[0], p_send_data_buf, data_len);
        g_req_i2c_send_data_len = data_len;
        g_idx_i2c_send_buf = 0;
        s_is_send_req = true;

        I2C_AcknowledgeConfig(I2C1, ENABLE);
        g_i2c_master_sate = I2C_STATE_START;
        I2C_GenerateSTART(I2C1, ENABLE);

        return I2C_RET_EXEC;
    } else {
        s_is_send_done = false;
        return I2C_RET_END;
    }
}

/**
 * @brief I2C 受信API
 * @param p_recv_data_buf 受信データバッファポインタ
 * @param data_len 受信したいデータバイト数
 * @return drv_i2c_ret処理結果
 */
drv_i2c_ret drc_i2c_recv(uint8_t *p_recv_data_buf, uint8_t data_len)
{
    if((p_recv_data_buf == NULL) || (data_len == 0)) {
        return I2C_RET_ERR;
    }

    if(s_is_recv_req == true) {
        return I2C_RET_BUSY;
    }

    if(s_is_recv_done != true) {
        memset((void *)&g_i2c_recv_buf[0], 0x00, I2C_RECV_BUF_SIZE);
        g_req_i2c_recv_data_len = data_len;
        g_idx_i2c_recv_buf = 0;
        s_is_recv_req = true;
        return I2C_RET_EXEC;
    } else {
        s_is_recv_done = false;
        return I2C_RET_END;
    }
}

//*********************************************************************