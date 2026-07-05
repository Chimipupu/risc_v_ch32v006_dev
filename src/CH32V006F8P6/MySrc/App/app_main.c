/**
 * @file app_main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-07-05
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_main.h"
#include "drv_gpio.h"
#include "drv_dma.h"
#include "drv_i2c.h"
#include "drv_tim.h"
#include "drv_i2c_eeprom_24c64.h"
#include "pcb_board_define.h"

#ifdef USE_APP_IO_REG
#include "app_io_reg.h"
#endif

#ifdef USE_74HC595
// 自前の74HC595ドライバ (https://github.com/Chimipupu/drv_74hc595.git)
#include "drv_74hc595.h"

#define SER_PIN    GPIO_PORT_D_4 // PD4: 74HC595 SERピン
#define SRCLK_PIN  GPIO_PORT_D_3 // PD3: 74HC595 SRCLKピン
#define RCLK_PIN   GPIO_PORT_D_2 // PD2: 74HC595 RCLKピン

static const drv_74hc595_config_t g_74hc595_cfg = {
    .ser_gpio_pin = SER_PIN,
    .srclk_gpio_pin = SRCLK_PIN,
    .rclk_gpio_pin = RCLK_PIN,
    .p_gpio_func = drv_gpio_port_onoff,
    .p_delay_ms_func = drv_tick_delay_ms
};
#endif

// -----------------------------------------------------------
// [DEBUG関連]
#define DEBUG_PRINTF        printf

#ifdef DEBUG_UART_USE
#include "dbg_mon.h"
#endif // DEBUG_UART_USE && DBG_MON_USE

#ifdef DEBUG_APP
// テストデータ（ASCII）: "CH32V006 DEVELOP BY CHIMIPUPU"
static const uint8_t g_test_ascii_tbl[32] = {
    0x43, 0x48, 0x33, 0x32, 0x56, 0x30, 0x30, 0x36,
    0x20, 0x44, 0x45, 0x56, 0x45, 0x4C, 0x4F, 0x50,
    0x42, 0x59, 0x20, 0x43, 0x48, 0x49, 0x4D, 0x49,
    0x50, 0x55, 0x50, 0x55, 0x20, 0x20, 0x20, 0x20
};
static void _dma_test(void);
#endif

#if defined(DEBUG_UART_USE) && defined(DBG_MON_USE)
static void _debug_proc(void);
#endif

// -----------------------------------------------------------

typedef struct {
    uint32_t chip_id_reg_val;
    const char *p_chip_type_str;
} chip_id_t;
static const chip_id_t g_chip_id_tbl[] = {
    {0x00600620, "CH32V006K8U6"},
    // {0x00610620, "CH32V006E8R6"},
    // {0x00620620, "CH32V006F8U6"},
    {0x00630620, "CH32V006F8P6"},
    // {0x00640620, "CH32V006F4U6"},
};
static const uint8_t g_chip_id_tbl_size = sizeof(g_chip_id_tbl) / sizeof(g_chip_id_tbl[0]);
uint32_t g_chip_id;
uint32_t g_chip_uid[3] = {0};

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
// uint8_t g_app_i2c_recv_data_buf[16] = {0};

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20)
const uint8_t g_aht20_cmd[3] = {0xAC, 0x33, 0x00};
#endif // I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280)
const uint8_t g_bmp280_reset_data[2] = {BMP280_REG_ADDR_RESET, BMP280_RESET_REG_EXP_VAL};
const uint8_t g_bmp280_id_reg_data[2] = {BMP280_REG_ADDR_ID, BMP280_ID_REG_EXP_VAL};
#endif //I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280

static void _i2c_sensor_read(void);

#if (I2C_RTC_DEVICE == I2C_RTC_RX8900)
const uint8_t g_dbg_i2c_send_data_buf[2] = {RTC_RX8900_REG_CTRL, 0x01};
#endif
static void rtc_time_read(void);

#if defined(DEBUG_I2C_USE) && defined(EEPROM_USE)
uint8_t g_eeprom_page_buf[EEPROM_24C64_PAGE_BYTE_SIZE] = {0};
// static bool _eeprom_factory_reset(void);
#endif // EEPROM_USE
#endif // DEBUG_I2C_USE

typedef uint8_t (*p_func_app_main)(void *p_arg);
static uint8_t _app_btn_proc(void *p_arg);

#ifdef USE_74HC595
// シリアル -> パラレル変換処理
static uint8_t _siri2para_proc(void *p_arg);
#endif

#ifdef USE_APP_IO_REG
static uint8_t _app_io_reg_proc(void *p_arg);
#endif

#ifdef DEBUG_I2C_USE
static uint8_t _i2c_proc(void *p_arg);
#endif // DEBUG_I2C_USE

typedef struct {
    p_func_app_main pfunc; // 各アプリのコールバック関数ポインタ
    uint16_t interval_ms;  // 各アプリの実行周期(ms)
} app_main_func_tbl_t;

// アプリメインコールバック関数テーブル
app_main_func_tbl_t g_app_func_tbl[] = {
#ifdef USE_74HC595
    {_siri2para_proc,  100}, // シリアル -> パラレル変換処理
#endif

    {_app_btn_proc,    300}, // ボタン処理アプリ

#ifdef DEBUG_I2C_USE
    {_i2c_proc,        1000}, // I2C処理
#endif // DEBUG_I2C_USE
};
#define APP_FUNC_TBL_CNT    sizeof(g_app_func_tbl) / sizeof(g_app_func_tbl[0])
static uint8_t s_idx = 0;
static uint8_t s_app_sw_timer_buf[APP_FUNC_TBL_CNT] = {0};

static void _steady_proc(void);
static void _period_proc(void);
// -----------------------------------------------------------
// [Static関数]

static void _steady_proc(void)
{
#ifdef USE_APP_IO_REG
    _app_io_reg_proc(); // I/Oレジスタアプリ
#endif

#if defined(DEBUG_UART_USE) && defined(DBG_MON_USE)
    _debug_proc(); // デバッグ処理
#endif
}

static void _period_proc(void)
{
    uint8_t cbK_ret;

    bool ret_sw_timer;

    ret_sw_timer = get_soft_timer_cnt_match(s_app_sw_timer_buf[s_idx]);

    if(ret_sw_timer == true) {
        // アプリ コールバック関数実行
        cbK_ret = g_app_func_tbl[s_idx].pfunc(NULL);

        if(cbK_ret != APP_PROC_EXEC) {
            // アプリ実行失敗時の処理
            if(cbK_ret == APP_PROC_ERROR) {
                // TODO
            }
        }
    }

    s_idx = (s_idx + 1) % APP_FUNC_TBL_CNT;
}

#ifdef DEBUG_APP
static void _dma_test(void)
{
    int cmp_ret;
    uint8_t dbg_dma_test_buf[32] = {0};
    drv_dma_config_t dma_cfg = {
        .data_type = DATA_TYPE_BYTE,
        .size_byte = 32,
        .p_src = (void *)&g_test_ascii_tbl[0],
        .p_dst = (void *)&dbg_dma_test_buf[0],
    };

    printf("[DMA Test] Start\r\n");
    printf("[DMA Test] Src Buf\r\n");
    app_util_mem_dump((const uint8_t *)&g_test_ascii_tbl[0], 32);

    drv_dma_init(DMA_CH_1, MODE_MEM2MEM, &dma_cfg);
    drv_dma_start(DMA_CH_1);

    while(drv_dma_tc_check(DMA_CH_1) == false);

    printf("[DMA Test] Dst Buf\r\n");
    app_util_mem_dump((const uint8_t *)&dbg_dma_test_buf[0], 32);

    cmp_ret = memcmp((const void *)&dbg_dma_test_buf, (const void *)&g_test_ascii_tbl, 32);
    if(cmp_ret == 0) {
        DEBUG_PRINTF("[DMA Test]] DMA Compare Succes!\r\n");
    } else {
        DEBUG_PRINTF("[DMA Test]] Error! DMA Compare Fail!\r\n");
    }
}
#endif

#if 0
// #if defined(DEBUG_I2C_USE) && defined(EEPROM_USE)
static bool _eeprom_factory_reset(void)
{
    bool ret;
    int ret_cmp;

    drv_eeprom_read_page(0x00, (uint8_t *)&g_eeprom_page_buf[0]);
    ret_cmp = memcmp((const void *)&g_test_ascii_tbl,
                     (const void *)&g_eeprom_page_buf,
                        EEPROM_24C64_PAGE_BYTE_SIZE);

    // EEPROMを工場出荷リセット
    if(ret_cmp != 0) {
        ret = true;
        drv_eeprom_write_page(0x00, (uint8_t *)&g_test_ascii_tbl[0]);
        drv_tick_delay_ms(10); // EEPROMの書き込み待ち時間の8ms以上待つ
        drv_eeprom_read_page(0x00, (uint8_t *)&g_eeprom_page_buf[0]);
        DEBUG_PRINTF("[DEBUG] EEPROM Factory Reset Done!\r\n");
    } else {
        ret = false;
        DEBUG_PRINTF("[DEBUG] This EEPROM Aleady Factory Reseted!\r\n");
    }

    // EEPORM メモリダンプ
    app_util_mem_dump((const uint8_t *)&g_eeprom_page_buf[0], EEPROM_24C64_PAGE_BYTE_SIZE);

    return ret;
}
#endif // EEPROM_USE

#ifdef DEBUG_I2C_USE
// NOTE: 浮動小数のfloatをprintf()できないので整数で処理して表示
static void _i2c_sensor_read(void)
{
    uint8_t tmp_u8 = 0;
    uint32_t tmp_u32 = 0;
    drv_i2c_ret drv_send_ret = I2C_RET_END;
    drv_i2c_ret drv_recv_ret = I2C_RET_END;

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_AHT20) // AHT20
    uint8_t aht20_read_buf[8] = {0};
    float aht20_humdity_data;
    float aht20_temp_data;

    // [AHT20から温度と湿度を読み出し]
    memset((uint8_t *)&aht20_read_buf[0], 0x00, 8);

    // AHT20に測定コマンドを送信(0xAC, 0x33, 0x00の順番)
    drv_send_ret = drv_i2c_write(I2C_ADDR_SENSOR_AHT20, (uint8_t *)&g_aht20_cmd[0], 3, true);

    // AHT20が測定完了するまで80ms以上待つ
    // drv_tick_delay_ms(100);

    // AHT20から6Byte一括読み出し
    drv_recv_ret = drv_i2c_read(I2C_ADDR_SENSOR_AHT20, (uint8_t *)&aht20_read_buf[0], 6, false, true);

    if(drv_recv_ret == I2C_RET_END) {
        // ステータスのBit7が1のBusyでないか?
        tmp_u8 = (aht20_read_buf[0]) & 0x80;
        if(tmp_u8 != 1) {
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

            if((drv_send_ret != I2C_RET_BUSY) && (drv_recv_ret != I2C_RET_BUSY)) {
                DEBUG_PRINTF("[DEBUG] AHT20: Temp = %d °C, Humdity = %d %%RH\r\n", (int32_t)aht20_temp_data, (uint32_t)aht20_humdity_data);
            }
        }
    }
#endif

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280) // BMP280
    uint8_t tx_data = 0;
    uint8_t bmp280_read_buf[8] = {0};
    float bmp280_temp_data;
    float bmp280_press_data;

    tx_data = BMP280_REG_ADDR_STATUS;
    drv_send_ret = drv_i2c_write(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&tx_data, 1, true);
    drv_recv_ret = drv_i2c_read(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&tmp_u8, 1, false, true);

    if((drv_recv_ret == I2C_RET_END)) {
        if(REG_BIT_CHK(tmp_u8, 3) != 1) {
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

                DEBUG_PRINTF("[DEBUG] BMP280: Temp = %d °C, Press = %d hPa\r\n", (int32_t)bmp280_temp_data, (int32_t)bmp280_press_data);
            }
        }
    }
#endif
}

static void rtc_time_read(void)
{
    uint8_t tx_data = 0;
    drv_i2c_ret drv_send_ret = I2C_RET_END;
    drv_i2c_ret drv_recv_ret = I2C_RET_END;
    uint8_t rtc_read_buf[16] = {0};

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

    if((drv_send_ret != I2C_RET_BUSY) && (drv_recv_ret != I2C_RET_BUSY)) {
        DEBUG_PRINTF("[DEBUG] RTC: %02X:%02X:%02X\r\n", rtc_read_buf[2], rtc_read_buf[1], rtc_read_buf[0]);
    }
}

static uint8_t _i2c_proc(void *p_arg)
{
    // DEBUG_PRINTF("[DEBUG] I2C Proc\r\n");

#if (I2C_ENV_SENSOR_DEVICE != I2C_ENV_SENSOR_NONE)
    _i2c_sensor_read(); // 環境センサー値取得 (温度、湿度)
#endif // I2C_ENV_SENSOR_DEVICE

#if (I2C_RTC_DEVICE != I2C_ENV_SENSOR_NONE)
    rtc_time_read(); // RTCから時刻取得
#endif // I2C_RTC_DEVICE

    return APP_PROC_END;
}
#endif // DEBUG_I2C_USE

static uint8_t _app_btn_proc(void *p_arg)
{
    if(g_is_btn_on_flg != false) {
        g_is_btn_on_flg = false;
        DEBUG_PRINTF("[DEBUG] EXTI0 IRQ (= PCB Button ON)\r\n");
    }

    return APP_PROC_END;
}

#ifdef USE_APP_IO_REG
static uint8_t _app_io_reg_proc(void *p_arg)
{
    uint8_t i, reg;

    for(i = 0; i < IO_REG_STR_TBL_CNT; i++)
    {
        reg = app_io_reg_read(g_io_reg_data_tbl[i].addr);
        DEBUG_PRINTF("[APP I/O Reg] %s, Addr: 0x%02X, Val: 0x%02X\r\n", g_io_reg_data_tbl[i].p_str,  g_io_reg_data_tbl[i].addr, reg);
    }

    return APP_PROC_END;
}
#endif

#if defined(DEBUG_UART_USE) && defined(DBG_MON_USE)
static void _debug_proc(void)
{
    dbg_mon_main(); // デバッグモニタ メイン
}
#endif

#ifdef USE_74HC595
// シリアル -> パラレル変換処理
static uint8_t _siri2para_proc(void *p_arg)
{
    static uint8_t s_siripara_out = 0;
    drv_74hc595_write_data_byte(s_siripara_out);
    s_siripara_out++;
}
#endif

// -----------------------------------------------------------
// [アプリ]

void app_util_mem_dump(const uint8_t *p_buf, uint32_t size_byte)
{
    uint32_t i;
    uint8_t c;
    uint32_t offset;
    uint32_t line_len;
    uint32_t addr;

    if (p_buf == NULL || size_byte == 0) {
        return;
    }

    addr = (uint32_t)(uintptr_t)(const void *)p_buf;

    printf("[Memory Dump] Addr: 0x%08X, Size: %u byte\r\n", addr, size_byte);
    printf("Addr      | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | ASCII\r\n");

    for (offset = 0; offset < size_byte; offset += 16)
    {
        addr += offset;
        printf("0x%08X| ", addr);

        line_len = (size_byte - offset < 16) ? (size_byte - offset) : 16;

        // HEX形式でダンプ
        for (i = 0; i < 16; i++)
        {
            if (i < line_len) {
                printf("%02X ", p_buf[offset + i]);
            } else {
                printf("   ");
            }
        }
        printf("|");

        // ASCIIでダンプ
        for (i = 0; i < line_len; i++)
        {
            c = p_buf[offset + i];
            // ASCIIは0x20(スペース) から 0x7E(~) までを表示
            c = ((c >= 0x20) && (c <= 0x7E)) ? c : '.';
            printf("%c", c);
        }
        printf("|\r\n");
    }
}

void app_util_print_mcu_chip_type(void)
{
    uint8_t i;
    bool is_chip_id_matched = false;

    g_chip_id = *((uint32_t *) REG_ADDR_CHIPID);

    for(i = 0; i < g_chip_id_tbl_size; i++)
    {
        if(g_chip_id_tbl[i].chip_id_reg_val == g_chip_id) {
            is_chip_id_matched = true;
            break;
        }
    }

    if(is_chip_id_matched) {
        DEBUG_PRINTF("MCU Chip Type: 0x%08X (%s)\r\n", g_chip_id, g_chip_id_tbl[i].p_chip_type_str);
    } else {
        DEBUG_PRINTF("Unknown MCU Chip Type\r\n");
    }
}

/**
 * @brief CH32V006のレジスタからユニークID(96bit)を読み出し
 * @return uint32_t* 読み出したUIDのバッファポインタ
 */
uint32_t* app_util_chip_uid_read(void)
{
    static bool s_is_uid_readed = false;
    uint32_t *p_ptr = (uint32_t *) &g_chip_uid[0];

    if(s_is_uid_readed != true) {
        memset((void *) &g_chip_uid[0], 0x00, sizeof(g_chip_uid));

        // UID 95~64 bit
        *p_ptr = *((uint32_t *) REG_ADDR_R32_ESIG_UNIID3);
        p_ptr++;
        // UID 63~32 bit
        *p_ptr = *((uint32_t *) REG_ADDR_R32_ESIG_UNIID2);
        p_ptr++;
        // UID 31~0 bit
        *p_ptr = *((uint32_t *) REG_ADDR_R32_ESIG_UNIID1);

        s_is_uid_readed = true;
    } else {
        DEBUG_PRINTF("Chip UID(96bit): 0x%08X 0x%08X 0x%08X\r\n", g_chip_uid[2], g_chip_uid[1], g_chip_uid[0]);
    }

    return p_ptr;
}

/**
 * @brief アプリメイン初期化
 */
void app_main_init(void)
{
#ifdef USE_APP_IO_REG
    app_io_reg_init(); // アプリI/Oレジスタ初期化
#endif

    DEBUG_PRINTF("PCB Info: Type = %s\r\n", PCB_NAME);
    DEBUG_PRINTF("CH32V006F8P6 Develop\r\n");
    app_util_print_mcu_chip_type();
    app_util_chip_uid_read(); // UID読み出し
    DEBUG_PRINTF("Clock: %d MHz\r\n", SystemCoreClock / 1000000);

#ifdef DEBUG_APP
    _dma_test(); // DMAテスト
#endif

// #ifdef EEPROM_USE
//     _eeprom_factory_reset(); // EEPROMの工場出荷リセット
// #endif // EEPROM_USE

#if (I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280)
    // BMP280 リセット
    drv_i2c_write(I2C_ADDR_SENSOR_BMP280, (uint8_t *)&g_bmp280_reset_data, 2, true);
#endif //I2C_ENV_SENSOR_DEVICE == I2C_ENV_SENSOR_BMP280

#if defined(DEBUG_UART_USE) && defined(DBG_MON_USE)
    dbg_mon_init(); // デバッグモニタ 初期化
#endif //DEBUG_UART_USE

#ifdef USE_74HC595
    // 自前の74HC595ドライバ (https://github.com/Chimipupu/drv_74hc595.git)
    drv_74hc595_init((drv_74hc595_config_t*) &g_74hc595_cfg);
#endif

#ifdef USE_SW_TIMER
    // S/Wタイマースタート
    for(uint8_t i = 0; i < APP_FUNC_TBL_CNT; i++)
    {
        soft_timer_start(g_app_func_tbl[i].interval_ms, true, &s_app_sw_timer_buf[i]);
    }
#endif
}

/**
 * @brief アプリメイン
 */
void app_main(void)
{
    _steady_proc(); // 定常処理
    _period_proc(); // 一定周期処理
}