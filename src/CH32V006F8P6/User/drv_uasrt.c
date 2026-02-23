/**
 * @file drv_uasrt.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 USARTドライバ＆ラッパー＆API
 * @version 0.1
 * @date 2025-08-06
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "drv_uasrt.h"
#include "app_main.h"

#ifdef DEBUG_UART_USE
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

static uint8_t s_rx_buf[USART_RX_BUF_SIZE] = {0}; // UASRT受信リングバッファ
static uint8_t s_rx_data_size = 0;                // 受信データサイズ
static uint8_t s_rx_buf_write_idx = 0;            // 受信バッファ書き込みインデックス
static uint8_t s_rx_buf_read_idx = 0;             // 受信バッファ読み出しインデックス
bool g_is_usart_irq_proc_end = false;

/**
 * @brief USART 割り込みハンドラ
 * 
 */
void USART1_IRQHandler(void)
{
    ITStatus tmp;

    tmp = USART_GetITStatus(USART1, USART_IT_RXNE);

    // 受信データをリングバッファに詰める
    while(tmp == SET)
    {
        s_rx_buf[s_rx_buf_write_idx] = (uint8_t)(USART1->DATAR & 0x00FF);
        s_rx_data_size = (s_rx_data_size + 1) % USART_RX_BUF_SIZE;
        s_rx_buf_write_idx = (s_rx_buf_write_idx + 1) % USART_RX_BUF_SIZE;
        tmp = USART_GetITStatus(USART1, USART_IT_RXNE);
    }

    g_is_usart_irq_proc_end = true;
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

/**
 * @brief get_char()と同じ機能のAPI
 * 
 * @return int32_t UART受信データ
 */
int32_t hw_usart_get_char(void)
{
    volatile int32_t val = 0;

    if(s_rx_data_size > 0) {
        val = (int32_t)s_rx_buf[s_rx_buf_read_idx];
        s_rx_buf_read_idx = (s_rx_buf_read_idx + 1) % USART_RX_BUF_SIZE;
        s_rx_data_size--;
    }

    return val;
}

void hw_usart_init(void)
{
    GPIO_InitTypeDef  gpio_pd5;
    GPIO_InitTypeDef  gpio_pd6;
    USART_InitTypeDef usart1;
    NVIC_InitTypeDef nvic;

    memset(&s_rx_buf, 0x00, sizeof(s_rx_buf));
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOD | RCC_PB2Periph_USART1, ENABLE);

    // TXピン (PD5)
    gpio_pd5.GPIO_Pin = GPIO_Pin_5;
    gpio_pd5.GPIO_Speed = GPIO_Speed_30MHz;
    gpio_pd5.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio_pd5);

    // RXピン (PD6)
    gpio_pd6.GPIO_Pin = GPIO_Pin_6;
    gpio_pd6.GPIO_Speed = GPIO_Speed_30MHz;
    gpio_pd6.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &gpio_pd6);

    // USART1を115200 8N1で初期化
    usart1.USART_BaudRate = 115200;
    usart1.USART_WordLength = USART_WordLength_8b;
    usart1.USART_StopBits = USART_StopBits_1;
    usart1.USART_Parity = USART_Parity_No;
    usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart1.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &usart1);

    // USARTの受信割り込み(RXNE)を有効
    USART1->STATR = 0x00C0;
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    USART_Cmd(USART1, ENABLE);
}
#endif