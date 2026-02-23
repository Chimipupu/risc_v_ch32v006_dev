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
volatile e_i2c_state master_sate = I2C_STATE_START;
volatile uint16_t master_recv_len = 0;
volatile uint16_t master_send_len = 0;
#else
volatile uint8_t slave_state = 0;
volatile uint16_t slave_recv_len = 0;
volatile uint16_t slave_send_len = 0;
#endif

void I2C1_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_ER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void I2C1_EV_IRQHandler(void )
{
#if (I2C_MODE == HOST_MODE)
    // I2C_IT_SB ... Start bit Flag (Host Mode)
    if( I2C_GetITStatus( I2C1, I2C_IT_SB ) != RESET ) {
        if(master_sate == I2C_STATE_START) {
            master_sate = I2C_STATE_AFTER_START;
            I2C_Send7bitAddress( I2C1, RTC_RX8900_I2C_SLAVE_ADDR, I2C_Direction_Transmitter );
        } else {
            master_sate = I2C_STATE_AFTER_REPEAT_START;
            I2C_Send7bitAddress( I2C1, RTC_RX8900_I2C_SLAVE_ADDR, I2C_Direction_Receiver );
        }
    }

    // I2C_IT_ADDR ... Address sent (Master Mode)
    else if( I2C_GetITStatus( I2C1, I2C_IT_ADDR ) != RESET ) {
        if(master_sate == I2C_STATE_AFTER_START) {
            master_sate = I2C_STATE_AFTER_WRITE_ADDR;
        }
        if(master_sate == I2C_STATE_AFTER_REPEAT_START) {
            master_sate = I2C_STATE_AFTER_READ_ADDR;
        }
        ((void)I2C_ReadRegister(I2C1, I2C_Register_STAR2));
    }

    // I2C_IT_TXE ... Data Register Empty (送信)
    else if( I2C_GetITStatus( I2C1, I2C_IT_TXE ) != RESET ) {
        if(master_sate == I2C_STATE_AFTER_WRITE_ADDR) {
            if(master_send_len < 6) {
                // I2C_SendData( I2C1, TxData[master_send_len] );
                master_send_len++;
            } else {
                master_sate = I2C_STATE_END_OF_SEND_DATA;
                I2C_GenerateSTART(I2C1, ENABLE);
                I2C_SendData( I2C1, 0xff ); // dummy byte, to prevent next TxE
            }
        }
    }

    // I2C_IT_RXNE ... Data Register Not Empty (受信)
    else if( I2C_GetITStatus( I2C1, I2C_IT_RXNE ) != RESET ) {
        if(I2C_STATE_AFTER_READ_ADDR) {
            if(master_recv_len < 6) {
                // RxData[master_recv_len] = I2C_ReceiveData(I2C1);
                master_recv_len++;
                if(master_recv_len == 5) {
                    I2C_NACKPositionConfig(I2C1,I2C_NACKPosition_Next); // clear ack
                    I2C_GenerateSTOP( I2C1, ENABLE );
                }
                if(master_recv_len == 6) {
                    master_sate = I2C_STATE_END;
                }
            } else {
                // TODO:
            }
        }
    }
#elif (I2C_MODE == SLAVE_MODE)
    // I2C_IT_ADDR ... Address matched flag (Slave mode)"ENDAD"
    if( I2C_GetITStatus( I2C1, I2C_IT_ADDR ) != RESET ) {
        if(I2C_GetFlagStatus( I2C1, I2C_FLAG_TRA )&&I2C_GetFlagStatus( I2C1, I2C_FLAG_TXE )) {
            //write mode
        } else {
            //read mode
            ((void)I2C_ReadRegister(I2C1, I2C_Register_STAR2));
        }
    }

    // I2C_IT_SB ... Start Bit
    else if( I2C_GetITStatus( I2C1, I2C_IT_SB ) != RESET ) {
        ((void)I2C_ReadRegister( I2C1, I2C_Register_STAR1));
        ((void)I2C_ReceiveData(I2C1));
    }

    // I2C_IT_TXE ... Data Register Empty
    else if( I2C_GetITStatus( I2C1, I2C_IT_TXE ) != RESET ) {
        I2C_SendData(I2C1,  RxData[slave_send_len]);
        slave_send_len++;
    }

    // I2C_IT_RXNE ... Data register not empty
    else if( I2C_GetITStatus( I2C1, I2C_IT_RXNE ) != RESET ) {
            RxData[slave_recv_len] = I2C_ReceiveData(I2C1);
            slave_recv_len++;
    }

    // I2C_IT_BTF ... Byte Transfer Finished flag
    else if( I2C_GetITStatus( I2C1, I2C_IT_BTF ) != RESET ) {
            ((void)I2C_ReadRegister( I2C1, I2C_Register_STAR1));
            ((void)I2C_ReceiveData(I2C1));
    }

    // I2C_IT_STOPF ... Data register empty flag
    else if( I2C_GetITStatus( I2C1, I2C_IT_STOPF ) != RESET ) {
        I2C1->CTLR1 &= I2C1->CTLR1;
        ((void)(I2C1->STAR1));
    }
#endif
    else{
        printf( "unknown i2c event \n" );
        printf("sr1 %x \nsr2 %x \n",I2C1->STAR1,I2C1->STAR2);
    }
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
        slave_state = 0xff;
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

    I2C_ITConfig( I2C1, I2C_IT_BUF, ENABLE );
    I2C_ITConfig( I2C1, I2C_IT_EVT, ENABLE );
    I2C_ITConfig( I2C1, I2C_IT_ERR, ENABLE );
    // ----------------------------------------------------------------------
}

//*********************************************************************