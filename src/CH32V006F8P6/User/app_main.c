/**
 * @file app_main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-02-25
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#include "app_main.h"
#include "app_io_reg.h"
#include "drv_i2c.h"
#include "drv_tim.h"
#include "drv_i2c_eeprom_24c64.h"
#include "drv_rtc_rx8900.h"
#include "pcb_board_define.h"
// -----------------------------------------------------------
// [DEBUG関連]

#if defined(DEBUG_UART_USE) && defined(DBG_COM_USE)
#include "dbg_com.h"
#endif // DEBUG_UART_USE && DBG_COM_USE

// -----------------------------------------------------------
// [Private]

#define APP_PROC_EXEC    0x00
#define APP_PROC_END     0x01

// アプリメイン用ステートマシーンの各処理ステップ
typedef enum {
    STEP_APP_INIT = 0x00, // 初期化ステップ
    STEP_APP_EXEC,        // 処理実行ステップ
    STEP_APP_RESULT       // 処理結果ステップ
} app_main_step;

#ifdef DEBUG_I2C_USE
    // I2C用アプリステートマシーンの各処理ステップ
    typedef enum {
        STEP_I2C_INIT = 0x00, // 初期化ステップ
        STEP_I2C_SEND,        // 送信ステップ
        STEP_I2C_RECV,        // 受信ステップ
        STEP_I2C_RESULT       // 処理結果ステップ
    } app_i2c_step;
    // volatile uint8_t g_app_i2c_recv_data_buf[16] = {0};

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20) || (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280) 
    #if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20)
    volatile const uint8_t g_aht20_cmd[3] = {0xAC, 0x33, 0x00};
    #endif // I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20

    #if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280)
    volatile const uint8_t g_bmp280_reset_data[2] = {BMP280_REG_ADDR_RESET, BMP280_RESET_REG_EXP_VAL};
    volatile const uint8_t g_bmp280_id_reg_data[2] = {BMP280_REG_ADDR_ID, BMP280_ID_REG_EXP_VAL};
    #endif //I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280

    static void env_sensor_read(void);
#endif

#if (I2C_RTC_DEVICE == I2C_RTC_DS3231) || (I2C_RTC_DEVICE == I2C_RTC_RX8900)
    #if (I2C_RTC_DEVICE == I2C_RTC_RX8900)
    volatile const uint8_t g_dbg_i2c_send_data_buf[2] = {RTC_RX8900_REG_CTRL, 0x01};
    #endif
    static void rtc_time_read(void);
#endif

    static void eeprom_read(void);
#endif // DEBUG_I2C_USE

volatile uint32_t g_chip_uid[3] = {0};

typedef uint8_t (*p_func_app_main)(void *p_arg);
#ifdef DEBUG_I2C_USE
static uint8_t _i2c_proc(void *p_arg);
#endif // DEBUG_I2C_USE
static uint8_t _debug_proc(void *p_arg);
static uint8_t _app_io_reg_proc(void *p_arg);

typedef struct {
    p_func_app_main pfunc; // アプリコールバック関数ポインタ
    uint16_t interval_ms;  // アプリコールバック関数の実行周期(ms)
} app_main_func_tbl_t;

// アプリコールバック関数テーブル
app_main_func_tbl_t g_app_func_tbl[] = {
    {_app_io_reg_proc, 900},
#ifdef DEBUG_I2C_USE
    {_i2c_proc,   1000},
#endif // DEBUG_I2C_USE
    {_debug_proc, 5000},
};
const uint8_t g_app_func_tbl_cnt = sizeof(g_app_func_tbl) / sizeof(g_app_func_tbl[0]);

static void util_chip_uid_read(uint32_t *p_buf);
// -----------------------------------------------------------
// [Static関数]
#ifdef DEBUG_I2C_USE
#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20) || (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280) 
static void env_sensor_read(void)
{
    volatile uint8_t tmp_u8;
    volatile uint32_t tmp_u32;
    volatile uint8_t tx_data = 0;
    volatile drv_i2c_ret drv_send_ret = I2C_RET_END;
    volatile drv_i2c_ret drv_recv_ret = I2C_RET_END;

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20) // AHT20
    volatile uint8_t aht20_read_buf[8] = {0};
    volatile float aht20_humdity_data;
    volatile float aht20_temp_data;

    // [AHT20から温度と湿度を読み出し]
    memset((uint8_t *)&aht20_read_buf[0], 0x00, 8);
    // AHT20に測定コマンドを送信(0xAC, 0x33, 0x00の順番)
    drv_send_ret = drv_i2c_write(I2C_ADDR_SENSOR_AHT20, (uint8_t *)&g_aht20_cmd[0], 3, true);
    // AHT20が測定完了するまで80ms以上待つ
    drv_tick_delay_ms(100);
    // AHT20から6Byte一括読み出し
    drv_recv_ret = drv_i2c_read(I2C_ADDR_SENSOR_AHT20, (uint8_t *)&aht20_read_buf[0], 6, false, true);
    if(drv_recv_ret == I2C_RET_END) {
        // ステータスのBit7が1のBusyでないか?
        if(REG_BIT_CHK(aht20_read_buf[0], 7) == 0) {
            // 20bit 湿度（0~100%RH ±2%RH）を取得
            tmp_u32 = ((uint32_t)aht20_read_buf[1]) << 12;          // 湿度のBit[19:12]
            tmp_u32 |= ((uint32_t)aht20_read_buf[2]) << 4;          // 湿度のBit[11:4]
            tmp_u32 |= (aht20_read_buf[3] >> 4);                    // 湿度のBit[3:0]
            aht20_humdity_data = ((float)tmp_u32 / 1048576.0f) * 100.0f;

            // 20bit 温度（-40~85°C ±0.3°C)を取得
            tmp_u32 = ((uint32_t)(aht20_read_buf[3] & 0x0F)) << 16; // 温度のBit[19:16]
            tmp_u32 |= ((uint32_t)aht20_read_buf[4]) << 8;          // 温度のBit[15:8]
            tmp_u32 |= ((uint32_t)aht20_read_buf[5]);               // 温度のBit[7:0]
            aht20_temp_data = ((float)tmp_u32 / 1048576.0f) * 200.0f - 50.0f;
        }
        #ifdef DEBUG_UART_USE
        printf("[DEBUG] AHT20: Temp = %d °C, Humdity = %d %%RH\r\n", (int32_t)aht20_temp_data, (uint32_t)aht20_humdity_data);
        #endif
#endif
    }

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280) // BMP280
    volatile uint8_t bmp280_read_buf[8] = {0};
    volatile float bmp280_temp_data;
    volatile float bmp280_press_data;

    tx_data = BMP280_REG_ADDR_STATUS;
    drv_send_ret = drv_i2c_write(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&tx_data, 1, true);
    drv_recv_ret = drv_i2c_read(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&tmp_u8, 1, false, true);

    if((drv_recv_ret == I2C_RET_END) && (REG_BIT_CHK(tmp_u8, 3) != 1)) {
        // [BMP280から温度と気圧を読み出し]
        memset((uint8_t *)&bmp280_read_buf[0], 0x00, sizeof(bmp280_read_buf));
        tx_data = BMP280_REG_ADDR_PRESS_MSB;
        drv_send_ret = drv_i2c_write(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&tx_data, 1, true);
        // BMP280から6Byte一括読み出し
        drv_recv_ret = drv_i2c_read(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&bmp280_read_buf[0], 6, false, true);
        if(drv_recv_ret == I2C_RET_END) {
            // 20bit 気圧（-40~85°C ±1.0°C)を取得
            tmp_u32 = ((uint32_t)bmp280_read_buf[0]) << 12;          // 温度のBit[19:12]
            tmp_u32 |= ((uint32_t)bmp280_read_buf[1]) << 4;          // 温度のBit[11:4]
            tmp_u32 |= (bmp280_read_buf[2] >> 4);                    // 温度のBit[3:0]
            bmp280_temp_data = (float)tmp_u32;

            // 20bit 気圧（300~1100hPa ±2hPa)を取得
            tmp_u32 = ((uint32_t)(bmp280_read_buf[3] & 0x0F)) << 16; // 気圧のBit[19:16]
            tmp_u32 |= ((uint32_t)bmp280_read_buf[4]) << 8;          // 気圧のBit[15:8]
            tmp_u32 |= ((uint32_t)bmp280_read_buf[5]);               // 気圧のBit[7:0]
            bmp280_press_data = (float)tmp_u32;

            #ifdef DEBUG_UART_USE
                printf("[DEBUG] BMP280: Temp = %d °C, Press = %d hPa\r\n", (int32_t)bmp280_temp_data, (int32_t)bmp280_press_data);
            #endif
        }
    }
#endif
}
#endif

#if (I2C_RTC_DEVICE == I2C_RTC_DS3231) || (I2C_RTC_DEVICE == I2C_RTC_RX8900)
static void rtc_time_read(void)
{
    volatile uint8_t tx_data = 0;
    volatile drv_i2c_ret drv_send_ret = I2C_RET_END;
    volatile drv_i2c_ret drv_recv_ret = I2C_RET_END;
    volatile uint8_t rtc_read_buf[16] = {0};

    memset((uint8_t *)&rtc_read_buf[0], 0x00, 16);
#if (I2C_RTC_DEVICE == I2C_RTC_DS3231)
    // [DS3231の全アドレス0x00~0x12を一括読み出し]
    drv_send_ret = drv_i2c_write(I2C_ADDR_RTC_DS3231, (uint8_t *)&tx_data, 1, true);
    drv_recv_ret = drv_i2c_read(I2C_ADDR_RTC_DS3231, (uint8_t *)&rtc_read_buf[0], 0x12, false, true);
#endif

#if (I2C_RTC_DEVICE == I2C_RTC_RX8900)
    // [RX8900の全アドレス0x00~0x0Fを一括読み出し]
    drv_send_ret = drv_i2c_write(I2C_ADDR_RTC_RX8900, (uint8_t *)&tx_data, 1, true);
    drv_recv_ret = drv_i2c_read(I2C_ADDR_RTC_RX8900, (uint8_t *)&rtc_read_buf[0], 0x0F, false, true);
#endif

#ifdef DEBUG_UART_USE
    if((drv_send_ret != I2C_RET_BUSY) && (drv_recv_ret != I2C_RET_BUSY)) {
        printf("[DEBUG] RTC: %02X:%02X:%02X\r\n", rtc_read_buf[2], rtc_read_buf[1], rtc_read_buf[0]);
    }
#endif
}
#endif

static void eeprom_read(void)
{
#if 0
    volatile uint8_t tx_data = 0;
    volatile drv_i2c_ret drv_send_ret = I2C_RET_END;
    volatile drv_i2c_ret drv_recv_ret = I2C_RET_END;
    volatile uint8_t eeprom_page_buf[EEPROM_24C64_PAGE_BYTE_SIZE] = {0};
    volatile uint16_t eeprom_addr = 0;

    memset((uint8_t *)&eeprom_page_buf[0], 0x00, EEPROM_24C64_PAGE_BYTE_SIZE);

    // TODO: (TBD) 詳細設計「詳細設計書_IOCPS」のシート「EEPROMメモリマップ」の読み出し
#endif
}

static uint8_t _i2c_proc(void *p_arg)
{
    printf("[DEBUG] I2C Proc\r\n");

#ifdef EEPROM_USE
    // eeprom_read(); // EEPROMからデータ読み出し
#endif // EEPROM_USE

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20) || (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280)
    env_sensor_read(); // 環境センサー値取得 (温度、湿度)
#endif // I2C_ENV_SENSOR_DEVICE

#if (I2C_RTC_DEVICE == I2C_RTC_DS3231) || (I2C_RTC_DEVICE == I2C_RTC_RX8900)
    rtc_time_read(); // RTCから時刻取得
#endif // I2C_RTC_DEVICE

    return APP_PROC_END;
}
#endif // DEBUG_I2C_USE

static uint8_t _app_io_reg_proc(void *p_arg)
{
    uint8_t i, reg;

    for(i = 0; i < IO_REG_STR_TBL_CNT; i++)
    {
        reg = app_io_reg_read(g_io_reg_str_tbl[i].addr);
        #ifdef DEBUG_UART_USE
            printf("[APP I/O Reg] %s, Addr: 0x%02X, Val: 0x%02X\r\n", g_io_reg_str_tbl[i].p_str,  g_io_reg_str_tbl[i].addr, reg);
        #endif    
    }

    return APP_PROC_END;
}

static uint8_t _debug_proc(void *p_arg)
{
    // printf("[DEBUG] Debug Proc\r\n");

#ifdef DEBUG_UART_USE
    printf("[DEBUG] PCB Info: Type = %s\r\n", PCB_NAME_STR);
    printf("[DEBUG] Chip UID(96bit): 0x%08X 0x%08X 0x%08X\r\n", g_chip_uid[2], g_chip_uid[1], g_chip_uid[0]);
#endif

#if defined(DEBUG_UART_USE) && defined(DBG_COM_USE)
    // デバッグモニタ メイン
    dbg_com_main();
#endif //DEBUG_UART_USE

    return APP_PROC_END;
}

/**
 * @brief マイコンのユニークID(96bit)読み出し
 * @param p_buf 96bit分のバッファポインタ(32bit x 3 = 96bit)
 */
static void util_chip_uid_read(uint32_t *p_buf)
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
// -----------------------------------------------------------
// [アプリ]

/**
 * @brief アプリメイン初期化
 */
void app_main_init(void)
{
    app_io_reg_init();                              // アプリI/Oレジスタ初期化
    util_chip_uid_read((uint32_t *)&g_chip_uid[0]); // 96bitのUID読み出し

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280)
    // BMP280 リセット
    drv_i2c_write(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&g_bmp280_reset_data, 2, true);
#endif //I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280

#if defined(DEBUG_UART_USE) && defined(DBG_COM_USE)
    dbg_com_init(); // デバッグモニタ 初期化
#endif //DEBUG_UART_USE
}

/**
 * @brief アプリメイン
 * @note 各アプリは実行周期が32bit SysTickタイマーのTickベースで実行
 */
void app_main(void)
{
    uint8_t cbK_ret;
    static uint32_t s_tick_cnt_ms[sizeof(g_app_func_tbl) / sizeof(g_app_func_tbl[0])] = {0};
    static uint32_t s_prev_tick_cnt = 0;
    uint32_t current_tick_cnt = drv_get_systick_cnt();
    uint32_t delta_ms = current_tick_cnt - s_prev_tick_cnt;

    s_prev_tick_cnt = current_tick_cnt;

    if(delta_ms == 0) {
        return;
    }

    for(uint8_t i = 0; i < g_app_func_tbl_cnt; i++)
    {
        s_tick_cnt_ms[i] += delta_ms;
        if(s_tick_cnt_ms[i] >= g_app_func_tbl[i].interval_ms) {
            cbK_ret = g_app_func_tbl[i].pfunc(NULL);
            if(cbK_ret == APP_PROC_END) {
                s_tick_cnt_ms[i] = 0;
            }
        }
    }
}