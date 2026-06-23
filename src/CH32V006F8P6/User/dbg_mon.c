/**
 * @file dbg_mon.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief デバッグモニタ(軽量板)
 * @version 0.1
 * @date 2026-03-26
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#include "dbg_mon.h"

// WCH SDK
#include <ch32v00X_it.h>

// MY App
#include "app_io_reg.h"
#include "app_main.h"
#include "pcb_board_define.h"
#include "drv_uart.h"
#include "drv_i2c_eeprom_24c64.h"

// -----------------------------------------------------------
// [DEBUG関連]
#ifdef DEBUG_DBG_MON
#endif // DEBUG_DBG_MON

// -----------------------------------------------------------
// バッファ関連
#define CMD_BUF_SIZE            32  // コマンドバッファサイズ
#define CMD_BUF_NUM             4   // コマンドバッファ段数

// コマンド構造体
typedef struct {
    const char *p_cmd_str;                   // コマンド
    const char *p_cmd_short_str;             // 短縮コマンド
    void (*p_func)(const uint8_t *p_args);   // コマンドコールバック関数ポインタ
    const char* p_description;               // コマンドの説明
} dbg_cmd_info_t;

#define MCU_NAME               "CH32V006F8P6"
#define PCB_NAME               "DEV PCB"
// #define PCB_NAME               "CH32V003F4P6-R0-1V1"
#define MCU_FLASH_SIZE         62
#define MCU_RAM_SIZE           8

static void dbg_mon_init_msg(const uint8_t *p_args);

static void cmd_help(const uint8_t *p_args);
static void cmd_rst(const uint8_t *p_args);
static void cmd_cls(const uint8_t *p_args);
static void cmd_system(const uint8_t *p_args);
static void cmd_mem_dump(const uint8_t *p_args);
static void cmd_reg(const uint8_t *p_args);
#ifdef EEPROM_USE
static void cmd_eeprom(const uint8_t *p_args);
#endif // EEPROM_USE

// コマンドテーブル
static const dbg_cmd_info_t g_cmd_tbl[] = {
//  | コマンド    | 短縮コマンド | コールバック関数   | コマンド説明 |
    // [システム関連コマンド]
    { "help",      "?",          &cmd_help,         "Show All Cmd"    },
    { "reset",     "rst",        &cmd_rst,          "Reset Cmd"       },
    { "clear",     "cls",        &cmd_cls,          "Display Clear"   },
    { "sysinfo",   "sif",        &cmd_system,       "Show SysInfo"    },
    { "memdump",   "mdp",        &cmd_mem_dump,     "MemDump Cmd"     },

    // [ペリフェラル関連コマンド]
    { "ioreg",     "irg",        &cmd_reg,          "I/O Reg R/W Cmd" },
#ifdef EEPROM_USE
    { "eeprom",    "e2p",         &cmd_eeprom,       "EEPROM R/W Cmd" },
#endif // EEPROM_USE
};
#define CMD_TBL_CNT    sizeof(g_cmd_tbl) / sizeof(g_cmd_tbl[0])

static void _cmd_exec(void);

// コマンドバッファ
static uint8_t s_cmd_buf[CMD_BUF_NUM][CMD_BUF_SIZE];
static uint8_t s_rx_buf_idx = 0;
static uint8_t s_rx_buf_number = 0;
static uint32_t s_rx_data_byte = 0;
// -----------------------------------------------------------
// [Static関数]

static void dbg_mon_init_msg(const uint8_t *p_args)
{
    printf("\nDebug Monior for %s Ver%d.%d.%d\n",  MCU_NAME,
                                                DBG_MON_VER_MAJOR,
                                                DBG_MON_VER_MINOR,
                                                DBG_MON_VER_REVISION);
    printf("Copyright (c) 2026 Chimipupu All Rights Reserved.\n");
}

static void cmd_rst(const uint8_t *p_args)
{
    printf("Now on Reset System! Will Be Restart.\r\n");
    NVIC_SystemReset(); // S/Wリセット
}

static void cmd_help(const uint8_t *p_args)
{
    dbg_mon_init_msg(p_args);

    printf("\nCmd Cnt: [%d]\n", CMD_TBL_CNT);
    for (uint8_t i = 0; i < CMD_TBL_CNT; i++)
    {
        printf("  %-10s | %-5s |  %s\n", g_cmd_tbl[i].p_cmd_str, g_cmd_tbl[i].p_cmd_short_str, g_cmd_tbl[i].p_description);
    }
}

static void cmd_cls(const uint8_t *p_args)
{
    printf(ANSI_ESC_CLS);
}

static void cmd_system(const uint8_t *p_args)
{
    uint32_t *p_uid_buf;

    printf("\n[System Info]\n");

    // 基板
    printf("\nPCB: %s\n", PCB_NAME);

    // マイコン
    // printf("MCU: %s\n", MCU_NAME);
    app_util_print_mcu_chip_type();
    printf("CPU: RISC-V RV32EmC (QingKe V2C)\n");
    printf("Clock: %d MHz\r\n", SystemCoreClock / 1000000);
    printf("Flash: %d KB\n", MCU_FLASH_SIZE);
    printf("SRAM: %d KB\n", MCU_RAM_SIZE);
    p_uid_buf = app_util_chip_uid_read(); // 96bit UID
    app_util_mem_dump((const uint8_t *) p_uid_buf, 12);
}

static void cmd_mem_dump(const uint8_t *p_args)
{
    // TODO
}

/**
 * @brief アプリ I/O レジスタR/Wコマンド関数
 * @param p_args コマンド引数
 */
static void cmd_reg(const uint8_t *p_args)
{
    // TODO
}

#ifdef EEPROM_USE
/**
 * @brief EEPROM R/Wコマンド関数
 * @param p_args コマンド引数
 * @note EEPROM ページ指定Read  ... 例 「e2p r p0」
 * @note EEPROM ページ指定Write: 未実装
 */
static void cmd_eeprom(const uint8_t *p_args)
{
    uint8_t i;
    uint8_t *p_ptr;
    uint8_t rw = '\0';
    uint16_t cmd_arg_e2p_page = 0;
    uint8_t e2p_page_buf[EEPROM_24C64_PAGE_BYTE_SIZE] = {0};

    // コマンド第1引数: 「e2p r」 or 「e2p w」の'r' or 'w'の部分
    if(s_cmd_buf[1][0] != '\0') {
        p_ptr = (uint8_t *) s_cmd_buf[1];
        rw = *p_ptr;
    }
    // エラー: 第1引数が'r'でも'w'でもない
    if((rw != 'r') && (rw != 'w')) {
        printf( ANSI_TXT_COLOR_RED    \
                "[ERROR] EEPROM Cmd Unknown Args: '%c' (must be 'r' or 'w')\r\n"    \
                ANSI_TXT_COLOR_RESET, rw);
        return;
    }

    // コマンド第2引数: ページ指定(xxx=0~255まで)
    if(s_cmd_buf[2][0] != '0') {
        p_ptr = (uint8_t *) s_cmd_buf[2];
        for(i = 0; i < CMD_BUF_SIZE; i++)
        {
            if((*p_ptr >= '0') && (*p_ptr <= '9')) {
                cmd_arg_e2p_page = (cmd_arg_e2p_page * 10) + (uint16_t)((*p_ptr - '0'));
            }

            p_ptr++;
        }
    }
    // エラー: 第2引数のページ指定がEEPROMのページ数以上
    if(cmd_arg_e2p_page > EEPROM_24C64_PAGE_NUM) {
        printf( ANSI_TXT_COLOR_RED    \
                "[ERROR] EEPROM Cmd, Page = %d, must be Page <= %d\r\n"    \
                ANSI_TXT_COLOR_RESET, cmd_arg_e2p_page, EEPROM_24C64_PAGE_NUM);
        return;
    }

    switch (rw)
    {
        // e2p r
        case 'r':
            printf("[DEBUG] EEPROM Read Cmd, Page [%d] Read\r\n", cmd_arg_e2p_page);
            drv_eeprom_read_page(cmd_arg_e2p_page, (uint8_t *)&e2p_page_buf[0]);
            app_util_mem_dump((const uint8_t *)&e2p_page_buf[0], EEPROM_24C64_PAGE_BYTE_SIZE);
            break;

        // e2p w
        case 'w':
            // TODO: EEPROMの書き込みコマンド対応
            printf( ANSI_TXT_COLOR_RED    \
                    "[ERROR] EEPROM Write Cmd, Not Support\r\n"    \
                    ANSI_TXT_COLOR_RESET);
            break;

        default:
            // NOP
            break;
    }
}
#endif // EEPROM_USE

static void _cmd_exec(void)
{
    uint8_t i;
    bool is_hit = false;

    // テーブルから該当コマンドを検索
    for(i = 0; i < CMD_TBL_CNT; i++)
    {
        if( (strcmp((const char *) s_cmd_buf[0], g_cmd_tbl[i].p_cmd_str) == 0) ||     // コマンドと一致か？
            (strcmp((const char *) s_cmd_buf[0], g_cmd_tbl[i].p_cmd_short_str) == 0)  // 短縮コマンドと一致か？
        ) {
            is_hit = true;
            break;
        }
    }

    if(is_hit != false) {
        g_cmd_tbl[i].p_func(NULL); // コマンド実行
    }
}

// -----------------------------------------------------------
// [API]

/**
 * @brief デバッグコマンドモニターの初期化
 */
void dbg_mon_init(void)
{
    memset(&s_cmd_buf[0], 0x00, sizeof(s_cmd_buf));
    s_rx_buf_idx = 0;

    // printf(ANSI_ESC_CLS);
    cmd_help(NULL);
    printf("\n>");
}

/**
 * @brief デバッグコマンドモニターのメイン処理
 */
void dbg_mon_main(void)
{
    uint8_t c;
    bool drv_ret;

    // UARTから1バイト受信
    drv_ret = drv_uart_get_char(&c);

    if(drv_ret == true) {
        // デリミタ(CRかLF)かヌル文字を受信でコマンド実行へ
        if((c == '\0') || (c == '\r') || (c == '\n')) {
            // コマンド実行
            if(s_rx_data_byte > 0) {
                _cmd_exec();
            }

            // バッファ関連メモリお掃除
            memset(&s_cmd_buf[0], 0x00, sizeof(s_cmd_buf));
            s_rx_buf_idx = 0;
            s_rx_buf_number = 0;
            s_rx_data_byte = 0;

            printf("\n>");
        }
        else if(c == ' ') {
            s_rx_buf_idx = 0;
            s_rx_buf_number = (s_rx_buf_number + 1) % CMD_BUF_NUM;
        }
        // バッファにデータを詰める更新
        else {
            s_cmd_buf[s_rx_buf_number][s_rx_buf_idx] = c;
            s_rx_buf_idx = (s_rx_buf_idx + 1) % CMD_BUF_SIZE;
            s_rx_data_byte++;
        }
    }
}