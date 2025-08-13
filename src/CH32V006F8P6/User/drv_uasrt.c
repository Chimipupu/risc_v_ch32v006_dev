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

void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

static uint8_t s_rx_buf[USART_RX_BUF_SIZE] = {0}; // UASRT受信リングバッファ
static uint8_t s_rx_data_size = 0;                // 受信データサイズ
static uint8_t s_rx_buf_write_idx = 0;            // 受信バッファ書き込みインデックス
static uint8_t s_rx_buf_read_idx = 0;             // 受信バッファ読み出しインデックス
static bool s_is_usart_irq_proc_end = false;

/**
 * @brief USART 割り込みハンドラ
 * 
 */
void USART1_IRQHandler(void)
{
    uint16_t tmp;

    tmp = USART1->STATR;
    tmp &= USART_STATR_RXNE;

    while(tmp == USART_STATR_RXNE){
        // 受信データをリングバッファに詰める
        s_rx_buf[s_rx_buf_write_idx] = (uint8_t)(USART1->DATAR & 0x00FF);
        s_rx_data_size = (s_rx_data_size + 1) % USART_RX_BUF_SIZE;
        s_rx_buf_write_idx = (s_rx_buf_write_idx + 1) % USART_RX_BUF_SIZE;

        tmp = USART1->STATR;
        tmp &= USART_STATR_RXNE;
    }

    s_is_usart_irq_proc_end = true;
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

/**
 * @brief USART受信ラッパー
 * 
 * @param p_val データポインタ
 * @return true 受信データあり
 * @return false 受信データなし
 */
bool hw_usart_get_byte(uint8_t *p_val)
{
    bool ret = false;
    volatile uint8_t val = 0;

    if (s_rx_data_size > 0) {
        val = s_rx_buf[s_rx_buf_read_idx];
        *p_val = val;
        s_rx_buf_read_idx = (s_rx_buf_read_idx + 1) % USART_RX_BUF_SIZE;
        s_rx_data_size--;
        ret = true;
    }

    return ret;
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

/**
 * @brief USART 受信文字列 表示関数
 * 
 */
void hw_usart_rx_data_print(void)
{
    bool ret;
    uint8_t val;
    uint16_t tx_val;

    ret = hw_usart_get_byte(&val);

    if(ret != false) {
        tx_val = (uint16_t)val;
        USART_SendData(USART1, tx_val);
        ret = false;
    }
}

/**
 * @brief USART受信ラッパー
 * 
 * @param p_val データポインタ
 * @return true 受信データあり
 * @return false 受信データなし
 */

/**
 * @brief get_char()と同じ機能のAPI
 * 
 * @return uint8_t 
 */
uint8_t hw_usart_get_char(void)
{
    volatile uint8_t val = 0;

    if((s_is_usart_irq_proc_end != false) && (s_rx_data_size > 0)) {
        s_is_usart_irq_proc_end = false;
        val = s_rx_buf[s_rx_buf_read_idx];
        s_rx_buf_read_idx = (s_rx_buf_read_idx + 1) % USART_RX_BUF_SIZE;
        s_rx_data_size--;
    }

    return val;
}