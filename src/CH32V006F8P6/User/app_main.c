/**
 * @file app_main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-02-25
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
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

extern bool g_is_tim_cnt_up;

volatile const uint8_t g_dbg_i2c_send_data_buf[2] = {RTC_RX8900_REG_CTRL, 0x01};
volatile uint8_t g_app_i2c_recv_data_buf[16] = {0};

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
    uint8_t ret = APP_PROC_EXEC;
    drv_i2c_ret drv_ret;
    static app_main_step s_step = 0;

    switch (s_step)
    {
        case STEP_INIT:
            drv_ret = drc_i2c_send((uint8_t *)&g_dbg_i2c_send_data_buf[0], 2);
            if(drv_ret == I2C_RET_END) {
                memset((void *)&g_app_i2c_recv_data_buf[0], 0x00, 16);
                s_step++;
            }
            break;

        case STEP_EXEC:
            drv_ret = drc_i2c_recv((uint8_t *)&g_app_i2c_recv_data_buf[0], 2);
            if(drv_ret == I2C_RET_END) {
                s_step++;
            }
            break;

        case STEP_RESULT:
            s_step = STEP_INIT;
            ret = APP_PROC_END;
            break;

        default:
            // NOP
            break;
    }

    return ret;
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