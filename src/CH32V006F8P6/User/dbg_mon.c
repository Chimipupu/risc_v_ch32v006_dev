/**
 * @file dbg_mon.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief デバッグモニタ(軽量板)
 * @version 0.1
 * @date 2026-03-26
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#include "dbg_mon.h"
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
// UART受信バッファのサイズ
#define DBG_CMD_UART_BUF_SIZE           128
// コマンドの最大長
#define DBG_CMD_MAX_LEN                 32
// コマンド引数バッファサイズ
#define DBG_CMD_ARGS_BUF_SIZE           (DBG_CMD_UART_BUF_SIZE - DBG_CMD_MAX_LEN)

// コマンド構造体
typedef struct {
    const char *p_cmd_str;                   // コマンド
    const char *p_cmd_short_str;             // 短縮コマンド
    void (*p_func)(const char *p_args);      // コマンドコールバック関数ポインタ
    const char* p_description;               // コマンドの説明
} dbg_cmd_info_t;

#define MCU_NAME               "CH32V006F8P6"
#define PCB_NAME               "DEV PCB"
// #define PCB_NAME               "CH32V003F4P6-R0-1V1"
#define MCU_FLASH_SIZE         62
#define MCU_RAM_SIZE           8

static void dbg_mon_init_msg(const char *p_args);

static void cmd_help(const char *p_args);
static void cmd_cls(const char *p_args);
static void cmd_system(const char *p_args);
static void cmd_mem_dump(const char *p_args);
static void cmd_reg(const char *p_args);
#ifdef EEPROM_USE
static void cmd_eeprom(const char *p_args);
#endif // EEPROM_USE
// コマンドテーブル
const dbg_cmd_info_t g_cmd_tbl[] = {
//  | コマンド    | 短縮コマンド | コールバック関数   | コマンド説明 |
    // [システム関連コマンド]
    { "help",      "?",          &cmd_help,         "Show All Cmd"    },
    { "clear",     "cls",        &cmd_cls,          "Display Clear"   },
    { "sysinfo",   "sif",        &cmd_system,       "Show SysInfo"    },
    { "memdump",   "mdp",        &cmd_mem_dump,     "MemDump Cmd"     },
    // [ペリフェラル関連コマンド]
    { "ioreg",     "irg",        &cmd_reg,          "I/O Reg R/W Cmd" },
#ifdef EEPROM_USE
    { "e2p",       "e2p",        &cmd_eeprom,       "EEPROM R/W Cmd" },
#endif // EEPROM_USE
};
#define CMD_TBL_CNT    sizeof(g_cmd_tbl) / sizeof(g_cmd_tbl[0])

static void _cmd_exec(const char *p_buf);

// UART受信バッファ関連
static char s_uart_recv_buf[DBG_CMD_UART_BUF_SIZE];
static uint8_t s_recv_buf_idx = 0;

// コマンド/コマンド引数バッファ
static char s_cmd_buf[DBG_CMD_MAX_LEN];
static char s_cmd_args_buf[DBG_CMD_ARGS_BUF_SIZE];

// -----------------------------------------------------------
// [Static関数]
static void dbg_mon_init_msg(const char *p_args)
{
    printf("\nDebug Monior for %s Ver%d.%d.%d\n",  MCU_NAME,
                                                DBG_MON_VER_MAJOR,
                                                DBG_MON_VER_MINOR,
                                                DBG_MON_VER_REVISION);
    printf("Copyright (c) 2026 Chimipupu All Rights Reserved.\n");
}

static void cmd_help(const char *p_args)
{
    dbg_mon_init_msg(p_args);

    printf("\nCmd Cnt: [%d]\n", CMD_TBL_CNT);
    for (uint8_t i = 0; i < CMD_TBL_CNT; i++)
    {
        printf("  %-10s | %-5s |  %s\n", g_cmd_tbl[i].p_cmd_str, g_cmd_tbl[i].p_cmd_short_str, g_cmd_tbl[i].p_description);
    }
}

static void cmd_cls(const char *p_args)
{
    printf(ANSI_ESC_CLS);
}

static void cmd_system(const char *p_args)
{
    // printf("\n[System Information]\n");

    // 基板
    printf("\nPCB: %s\n", PCB_NAME);

    // マイコン
    printf("MCU: %s\n", MCU_NAME);
    printf("CPU: RISC-V RV32EmC (QingKe V2C)\n");

    // ROM/RAM
    printf("Flash: %d KB\n", MCU_FLASH_SIZE);
    printf("SRAM: %d KB\n", MCU_RAM_SIZE);

    // クロック関連
    printf("System Clock: %d MHz\r\n", SystemCoreClock / 1000000);
}

static void cmd_mem_dump(const char *p_args)
{
    // TODO
}

/**
 * @brief アプリ I/O レジスタR/Wコマンド関数
 * @param p_args コマンド引数
 */
static void cmd_reg(const char *p_args)
{
    // TODO
}

#ifdef EEPROM_USE
/**
 * @brief EEPROM R/Wコマンド関数
 * @param p_args コマンド引数
 * @note EEPROM ページ指定Read  ... 例 「e2p r page=0」
 * @note EEPROM ページ指定Write: 未実装
 */
static void cmd_eeprom(const char *p_args)
{
    uint8_t *p_ptr;
    uint8_t rw;
    uint16_t cmd_arg_e2p_page = 0;
    uint8_t e2p_page_buf[EEPROM_24C64_PAGE_BYTE_SIZE] = {0};

    p_ptr = p_args;

    // コマンド引数: 「e2p r」 or 「e2p w」の部分
    rw = *p_ptr; // 'r' or 'w'

    // 「page=xx」までポインタを進める
    p_ptr += 6;

    // コマンド引数: 「page=xx」の「xx」部分
    while((*p_ptr != '\0') && (*p_ptr != '\r') && (*p_ptr != '\n'))
    {
        if((*p_ptr >= '0') && (*p_ptr <= '9')) {
            cmd_arg_e2p_page = (cmd_arg_e2p_page * 10) + (uint16_t)((*p_ptr - '0'));
        }
        p_ptr++;
    }

    // エラー: 指定ページ数がEEPROMのページ数以上
    if(cmd_arg_e2p_page > EEPROM_24C64_PAGE_NUM) {
        printf( ANSI_TXT_COLOR_RED    \
                "[ERROR] EEPROM Cmd, Page = %d, must be Page > %d\r\n"    \
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

static void _cmd_exec(const char *p_buf)
{
    uint8_t i;
    uint8_t *p_ptr = NULL;
    uint8_t cmd_buf_idx = 0;
    uint8_t cmd_args_buf_idx = 0;
    bool is_cmd_recv = true;

    if(p_buf == NULL) {
        return;
    }

    p_ptr = (uint8_t *)p_buf;

    // コマンド解析
    for(i = 0; i < DBG_CMD_UART_BUF_SIZE; i++)
    {
        // デリミタ(CR or LF)かヌル文字が来たら、もう何もコマンド関連のデータが無いからここで終了
        if((*p_ptr == '\r') || (*p_ptr == '\n') || (*p_ptr == '\0')) {
            break; // Break For Loop
        }
        else if(*p_ptr == ' ') {
            // スペースが来る時点でもうコマンドちゃうからフラグを降ろしとく
            is_cmd_recv = false;
        } else {
            if(is_cmd_recv != false) {
                // コマンドを格納 (受信バッファの冒頭からスペースの手前まで)
                s_cmd_buf[cmd_buf_idx] = *p_ptr;
                cmd_buf_idx = (cmd_buf_idx + 1) % DBG_CMD_MAX_LEN;
            } else {
                // コマンド引数を格納 (受信バッファのスペースの後～残りの部分まで)
                s_cmd_args_buf[cmd_args_buf_idx] = *p_ptr;
                cmd_args_buf_idx = (cmd_args_buf_idx + 1) % DBG_CMD_ARGS_BUF_SIZE;
            }
        }

        // 次のデータへ
        p_ptr++;
    }


    // テーブルから該当コマンドのコールバック関数を検索
    for(i = 0; i < CMD_TBL_CNT; i++)
    {
        if( (strcmp(&s_cmd_buf[0], g_cmd_tbl[i].p_cmd_str) == 0) ||     // コマンドと一致か？
            (strcmp(&s_cmd_buf[0], g_cmd_tbl[i].p_cmd_short_str) == 0)  // 短縮コマンドと一致か？
        ) {
#ifdef DEBUG_DBG_MON
            printf("[DEBUG] cmd: %s\r\n", s_cmd_buf);
            printf("[DEBUG] cmd agrs: %s\r\n", s_cmd_args_buf);
#endif // DEBUG_DBG_MON

            // コマンド実行
            g_cmd_tbl[i].p_func(&s_cmd_args_buf[0]);
        }
    }
}

// -----------------------------------------------------------
// [API]

/**
 * @brief デバッグコマンドモニターの初期化
 */
void dbg_mon_init(void)
{
    memset(&s_uart_recv_buf[0], 0x00, DBG_CMD_UART_BUF_SIZE);
    memset(&s_cmd_buf[0], 0x00, DBG_CMD_MAX_LEN);
    memset(&s_cmd_args_buf[0], 0x00, DBG_CMD_ARGS_BUF_SIZE);

    // printf(ANSI_ESC_CLS);
    cmd_help(NULL);
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
    if(drv_ret == false) {
        return;
    }

    // リングバッファ更新
    s_uart_recv_buf[s_recv_buf_idx] = c;
    s_recv_buf_idx = (s_recv_buf_idx + 1) % DBG_CMD_MAX_LEN;

    // デリミタ'CRかLF)の受信でコマンド受付け
    if ((c == '\r') || (c == '\n')) {
        // 何かしらデータが受信出来てたら処理
        if (s_recv_buf_idx > 3) {
            printf("\n");

            // コマンド解析 & 実行
            _cmd_exec((const char *)&s_uart_recv_buf[0]);

            // バッファ関連お掃除
            memset(&s_uart_recv_buf[0], 0x00, DBG_CMD_UART_BUF_SIZE);
            s_recv_buf_idx = 0;
            memset(&s_cmd_buf[0], 0x00, DBG_CMD_MAX_LEN);
            memset(&s_cmd_args_buf[0], 0x00, DBG_CMD_ARGS_BUF_SIZE);
        }

        printf("\n> ");
    }
}