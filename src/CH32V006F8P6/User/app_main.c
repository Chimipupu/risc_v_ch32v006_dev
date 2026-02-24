/**
 * @file app_main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2025-06-13
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#include "app_main.h"
#include "drv_i2c.h"
#include "drv_tim.h"
#include "drv_rtc_rx8900.h"
// -----------------------------------------------------------
// [DEBUG関連]

#ifdef DEBUG_APP
volatile uint16_t g_dbg_start_timer_cnt = 0;
volatile uint16_t g_dbg_end_timer_cnt = 0;
#endif // DEBUG_APP

#ifdef DEBUG_UART_USE
#include "dbg_com.h"
extern bool g_is_usart_irq_proc_end;
#endif

// -----------------------------------------------------------
// [Private]

#define APP_PROC_EXEC    0x00
#define APP_PROC_END     0x01

#if (I2C_MODE == HOST_MODE)
extern volatile e_i2c_state g_i2c_master_sate;
extern uint8_t g_i2c_send_buf[I2C_SEND_BUF_SIZE];
extern uint8_t g_i2c_recv_buf[I2C_RECV_BUF_SIZE];
#else
extern volatile uint8_t g_i2c_slave_state;
extern volatile uint16_t g_i2c_slave_recv_len;
extern volatile uint16_t g_i2c_slave_send_len;
#endif

volatile uint8_t g_req_i2c_send_data_len; // I2Cで送信したいデータ数
volatile uint8_t g_req_i2c_recv_data_len; // I2Cで受信したいデータ数

extern bool g_is_tim_cnt_up;

typedef uint8_t (*p_func_app_main)(void *p_arg);
static uint8_t _i2c_proc(void *p_arg);
static uint8_t _debug_proc(void *p_arg);

p_func_app_main g_app_func_tbl[] = {
    _i2c_proc,
    _debug_proc,
};
const uint8_t g_app_func_tbl_cnt = sizeof(g_app_func_tbl) / sizeof(g_app_func_tbl[0]);

static uint8_t s_func_tbl_idx = 0;
// -----------------------------------------------------------
// [Static関数]

static uint8_t _i2c_proc(void *p_arg)
{
    if (g_i2c_master_sate == I2C_STATE_END || g_i2c_master_sate == I2C_STATE_START) {
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == RESET) {
            memset(g_i2c_send_buf, 0x00, I2C_SEND_BUF_SIZE);
            memset(g_i2c_recv_buf, 0x00, I2C_RECV_BUF_SIZE);
            g_req_i2c_send_data_len = 16;
            g_req_i2c_recv_data_len = 16;
            g_i2c_send_buf[0] = RTC_RX8900_REG_SEC         ;
            g_i2c_send_buf[1] = RTC_RX8900_REG_MIN         ;
            g_i2c_send_buf[2] = RTC_RX8900_REG_HOUR        ;
            g_i2c_send_buf[3] = RTC_RX8900_REG_WEEK        ;
            g_i2c_send_buf[4] = RTC_RX8900_REG_DAY         ;
            g_i2c_send_buf[5] = RTC_RX8900_REG_MONTH       ;
            g_i2c_send_buf[6] = RTC_RX8900_REG_YEAR        ;
            g_i2c_send_buf[7] = RTC_RX8900_REG_RAM         ;
            g_i2c_send_buf[8] = RTC_RX8900_REG_MIN_ALM     ;
            g_i2c_send_buf[9] = RTC_RX8900_REG_HOUR_ALM    ;
            g_i2c_send_buf[10] = RTC_RX8900_REG_WEEK_DAY_ALM;
            g_i2c_send_buf[11] = RTC_RX8900_REG_TIMER_CNT_0 ;
            g_i2c_send_buf[12] = RTC_RX8900_REG_TIMER_CNT_1 ;
            g_i2c_send_buf[13] = RTC_RX8900_REG_EXTENSION   ;
            g_i2c_send_buf[14] = RTC_RX8900_REG_FLAG        ;
            g_i2c_send_buf[15] = RTC_RX8900_REG_CTRL        ;
            I2C_AcknowledgeConfig(I2C1, ENABLE);
            g_i2c_master_sate = I2C_STATE_START;
            I2C_GenerateSTART(I2C1, ENABLE);
        }
    }

    return APP_PROC_END;
}

static uint8_t _debug_proc(void *p_arg)
{
#ifdef DEBUG_UART_USE
    // デバッグモニタ メイン
    dbg_com_main();
#endif //DEBUG_UART_USE

    return APP_PROC_END;
}

// -----------------------------------------------------------
// [API]

/**
 * @brief アプリメイン初期化
 */
void app_main_init(void)
{
#ifdef DEBUG_UART_USE
    // デバッグモニタ 初期化
    dbg_com_init();
#endif //DEBUG_UART_USE
}

/**
 * @brief アプリメイン
 */
void app_main(void)
{
    uint8_t ret;

#ifdef DEBUG_APP
    // g_dbg_start_timer_cnt = drv_get_tim_cnt();
#endif // DEBUG_APP

    // タイマーがカウントUP ... 65.535ms
    if(g_is_tim_cnt_up != false) {
        g_is_tim_cnt_up = false;
    }

    // アプリのコールバック関数実行
    ret = g_app_func_tbl[s_func_tbl_idx](NULL);
    if(ret == APP_PROC_END) {
        s_func_tbl_idx = (s_func_tbl_idx + 1) % g_app_func_tbl_cnt;
    }

#ifdef DEBUG_APP
    // g_dbg_end_timer_cnt = drv_get_tim_cnt();
#endif // DEBUG_APP
}