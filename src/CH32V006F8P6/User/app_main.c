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
// volatile uint16_t g_dbg_start_timer_cnt = 0;
// volatile uint16_t g_dbg_end_timer_cnt = 0;
#endif // DEBUG_APP

#if defined(DEBUG_UART_USE) && defined(DBG_COM_USE)
#include "dbg_com.h"
#endif

// -----------------------------------------------------------
// [Private]

#define APP_PROC_EXEC    0x00
#define APP_PROC_END     0x01

// アプリメイン用ステートマシーンの各処理ステップ
typedef enum {
    STEP_I2C_INIT = 0x00, // 初期化ステップ
    STEP_I2C_SEND,        // 送信ステップ
    STEP_I2C_RECV,        // 受信ステップ
    STEP_I2C_RESULT       // 処理結果ステップ
} app_i2c_step;

extern bool g_is_tim_cnt_up;

volatile const uint8_t g_dbg_i2c_send_data_buf[2] = {RTC_RX8900_REG_CTRL, 0x01};
volatile uint8_t g_app_i2c_recv_data_buf[16] = {0};
volatile uint32_t g_chip_uid[3] = {0};

typedef uint8_t (*p_func_app_main)(void *p_arg);
static uint8_t _i2c_proc(void *p_arg);
static uint8_t _debug_proc(void *p_arg);

// アプリコールバック関数テーブル
p_func_app_main g_app_func_tbl[] = {
    _i2c_proc,
    _debug_proc,
};
const uint8_t g_app_func_tbl_cnt = sizeof(g_app_func_tbl) / sizeof(g_app_func_tbl[0]);

static uint8_t s_func_tbl_idx = 0;
// -----------------------------------------------------------
// [Static関数]
volatile const uint8_t g_aht20_cmd[3] = {0xAC, 0x33, 0x00};

static uint8_t _i2c_proc(void *p_arg)
{
    volatile uint8_t tmp_u8;
    volatile uint32_t tmp_u32;
    volatile uint8_t tx_data = 0;
    volatile uint8_t rtc_read_buf[16] = {0};
    volatile uint8_t aht20_read_buf[8] = {0};
    volatile float aht20_humdity_data;
    volatile float aht20_temp_data;
    volatile drv_i2c_ret drv_send_ret = I2C_RET_END;
    volatile drv_i2c_ret drv_recv_ret = I2C_RET_END;

#if 1
    // [AHT20から温度と湿度を読み出し]
    memset((uint8_t *)&aht20_read_buf[0], 0x00, 8);
    // AHT20に測定コマンドを送信(0xAC, 0x33, 0x00の順番)
    drv_send_ret = drc_i2c_send(I2C_ADDR_SENSOR_AHT20, (uint8_t *)&g_aht20_cmd[0], 3);
    // AHT20が測定完了するまで80ms以上待つ
    Delay_Ms(100);
    // AHT20から6Byte一括読み出し
    drv_recv_ret = drc_i2c_recv(I2C_ADDR_SENSOR_AHT20, (uint8_t *)&aht20_read_buf[0], 6, false);
    if(drv_recv_ret == I2C_RET_END) {
        // ステータスのBit7が1のBusyでないか?
        if(REG_BIT_CHK(aht20_read_buf[0], 7) == 0) {
            // 20bit 湿度を取得
            tmp_u32 = ((uint32_t)aht20_read_buf[1]) << 12;        // 湿度のBit[19:12]
            tmp_u32 |= ((uint32_t)aht20_read_buf[2]) << 4;        // 湿度のBit[11:4]
            tmp_u32 |= (aht20_read_buf[3] >> 4);                  // 湿度のBit[3:0]
            aht20_humdity_data = ((float)tmp_u32 / 1048576.0f) * 100.0f;

            // 20bit 温度を取得
            tmp_u32 = ((uint32_t)(aht20_read_buf[3] & 0x0F)) << 16; // 温度のBit[19:16]
            tmp_u32 |= ((uint32_t)aht20_read_buf[4]) << 8;          // 温度のBit[15:8]
            tmp_u32 |= ((uint32_t)aht20_read_buf[5]);               // 温度のBit[7:0]
            aht20_temp_data = ((float)tmp_u32 / 1048576.0f) * 200.0f - 50.0f;
        }
    }
    #ifdef DEBUG_UART_USE
        // floatの温度と湿度の整数部分だけpintf()
        printf("[DEBUG] AHT20: Temp = %d C, Humdity = %d %%\r\n", (uint32_t)aht20_temp_data, (uint32_t)aht20_humdity_data);
    #endif
#endif

    memset((uint8_t *)&rtc_read_buf[0], 0x00, 16);
#if 1
    // [DS3231の全アドレス0x00~0x12を一括読み出し]
    drv_send_ret = drc_i2c_send(I2C_ADDR_RTC_DS3231, (uint8_t *)&tx_data, 1);
    drv_recv_ret = drc_i2c_recv(I2C_ADDR_RTC_DS3231, (uint8_t *)&rtc_read_buf[0], 0x12, false);
#else
    // [RX8900の全アドレス0x00~0x0Fを一括読み出し]
    drv_send_ret = drc_i2c_send(I2C_ADDR_RTC_RX8900, (uint8_t *)&tx_data, 1);
    drv_recv_ret = drc_i2c_recv(I2C_ADDR_RTC_RX8900, (uint8_t *)&rtc_read_buf[0], 0x0F, false);
#endif

#ifdef DEBUG_UART_USE
    if((drv_send_ret != I2C_RET_BUSY) && (drv_recv_ret != I2C_RET_BUSY)) {
        printf("[DEBUG] RTC: %02X:%02X:%02X\r\n", rtc_read_buf[2], rtc_read_buf[1], rtc_read_buf[0]);
        Delay_Ms(1000);
    }
#endif

    return APP_PROC_END;
}

static uint8_t _debug_proc(void *p_arg)
{
#if defined(DEBUG_UART_USE) && defined(DBG_COM_USE)
    // デバッグモニタ メイン
    dbg_com_main();
#endif //DEBUG_UART_USE

    return APP_PROC_END;
}

// -----------------------------------------------------------
// [API]

/**
 * @brief マイコンのユニークID(96bit)読み出し
 * @param p_buf 96bit分のバッファポインタ(32bit x 3 = 96bit)
 */
void util_chip_uid_read(uint32_t *p_buf)
{
    uint32_t *p_ptr;

    if(p_buf == NULL) {
        return;
    }

    p_ptr = p_buf;

    // UID 0~31 bit
    *p_ptr = *((uint32_t *) REG_ADDR_R32_ESIG_UNIID1);
    p_ptr++;
    // UID 32~63 bit
    *p_ptr = *((uint32_t *) REG_ADDR_R32_ESIG_UNIID2);
    p_ptr++;
    // UID 64~95bit
    *p_ptr = *((uint32_t *) REG_ADDR_R32_ESIG_UNIID3);
}

/**
 * @brief アプリメイン初期化
 */
void app_main_init(void)
{
    // 96bitのUID読み出し
    util_chip_uid_read((uint32_t *)&g_chip_uid[0]);
#ifdef DEBUG_UART_USE
    printf("[DEBUG] Chip UID(96bit): 0x%08X 0x%08X 0x%08X\r\n", g_chip_uid[2], g_chip_uid[1], g_chip_uid[0]);
#endif

#if defined(DEBUG_UART_USE) && defined(DBG_COM_USE)
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