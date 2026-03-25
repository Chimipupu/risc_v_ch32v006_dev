/**
 * @file dbg_com.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief デバッグUARTモニタ
 * @version 0.1
 * @date 2026-03-26
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#include "dbg_com.h"

// -----------------------------------------------------------
// [DEBUG関連]

// -----------------------------------------------------------
// コマンド関連のマクロ
#define DBG_CMD_UART_BUF_SIZE           128 // UART受信バッファのサイズ
#define DBG_CMD_MAX_LEN                 32  // コマンドの最大長

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

static void dbg_com_init_msg(const char *p_args);

static void cmd_help(const char *p_args);
static void cmd_cls(const char *p_args);
static void cmd_system(const char *p_args);
static void cmd_mem_dump(const char *p_args);
static void cmd_reg(const char *p_args);
// コマンドテーブル
const dbg_cmd_info_t g_cmd_tbl[] = {
//  | コマンド    | 短縮コマンド | コールバック関数   | コマンド説明 |
    { "help",      "?",          &cmd_help,         "Show All Cmd"},
    { "clear",     "cls",        &cmd_cls,          "Display Clear"},
    { "sysinfo",   "sif",        &cmd_system,       "Show SysInfo"},
    { "memdump",   "md",         &cmd_mem_dump,     "MemDump Cmd"},
    { "ioreg",     "ir",         &cmd_reg,          "I/O Reg R/W Cmd"},
};
#define CMD_TBL_CNT    sizeof(g_cmd_tbl) / sizeof(g_cmd_tbl[0])

static void _cmd_exec(const char *p_buf);

// コマンドバッファ
static char s_uart_recv_buf[DBG_CMD_UART_BUF_SIZE];
static char s_cmd_buf[DBG_CMD_MAX_LEN];
static uint8_t s_buf_idx = 0;

// -----------------------------------------------------------
// [Static関数]

static void dbg_com_init_msg(const char *p_args)
{
    printf("\nDebug Cmd for %s Ver%d.%d.%d\n",  MCU_NAME,
                                                DBG_COM_VER_MAJOR,
                                                DBG_COM_VER_MINOR,
                                                DBG_COM_VER_REVISION);
    printf("Copyright (c) 2026 Chimipupu All Rights Reserved.\n");
}

static void cmd_help(const char *p_args)
{
    dbg_com_init_msg(p_args);

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
 * @param p_args コマンド引数の構造体ポインタ
 */
static void cmd_reg(const char *p_args)
{
    // TODO
}

static void _cmd_exec(const char *p_buf)
{
    uint8_t i;
    uint8_t *p_ptr;

    if(p_buf == NULL) {
        return;
    }

    p_ptr = (uint8_t *)p_buf;

    // コマンド解析
    for(i = 0; i < DBG_CMD_UART_BUF_SIZE; i++)
    {
        if((*p_ptr == '\r') || (*p_ptr == '\n') || (*p_ptr == '\0')) {
            break; // Break For Loop
        } else {
            s_cmd_buf[i] = *p_ptr;
            p_ptr++;
        }
    }

    // コマンド実行
    for(i = 0; i < CMD_TBL_CNT; i++)
    {
        if( (strcmp(&s_cmd_buf[0], g_cmd_tbl[i].p_cmd_str) == 0) ||     // コマンドと一致か？
            (strcmp(&s_cmd_buf[0], g_cmd_tbl[i].p_cmd_short_str) == 0)  // 短縮コマンドと一致か？
        ) {
            g_cmd_tbl[i].p_func(NULL);
        }
    }
}

// -----------------------------------------------------------
// [API]

/**
 * @brief デバッグコマンドモニターの初期化
 */
void dbg_com_init(void)
{
    s_buf_idx = 0;
    // printf(ANSI_ESC_CLS);
    cmd_help(NULL);
}

/**
 * @brief デバッグコマンドモニターのメイン処理
 */
void dbg_com_main(void)
{
    uint8_t c;
    bool drv_ret;

    // UARTから1バイト受信
    drv_ret = drv_uart_get_char(&c);
    if(drv_ret == false) {
        return;
    }

    // リングバッファ更新
    s_uart_recv_buf[s_buf_idx] = c;
    s_buf_idx = (s_buf_idx + 1) % DBG_CMD_MAX_LEN;

    // デリミタでCRかLFが来てたらコマンドを受付ける
    if ((c == '\r') || (c == '\n')) {
        if (s_buf_idx > 0) {
            printf("\n");

            // コマンド解析 & 実行
            _cmd_exec((const char *)&s_uart_recv_buf[0]);
        }

        memset(&s_uart_recv_buf[0], 0x00, DBG_CMD_UART_BUF_SIZE);
        memset(&s_cmd_buf[0], 0x00, DBG_CMD_MAX_LEN);
        s_buf_idx = 0;
        printf("\n> ");
    }
}