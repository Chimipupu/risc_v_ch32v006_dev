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
// コマンド引数構造体
typedef struct {
    int32_t argc;                    // 引数の数
    char* p_argv[DBG_CMD_MAX_ARGS];  // 引数の配列
} dbg_cmd_args_t;

// コマンド構造体
typedef struct {
    const char* p_cmd_str;                   // コマンド文字列
    void (*p_func)(dbg_cmd_args_t *p_args);  // コールバック関数ポインタ
    int32_t min_args;                        // 最小引数数
    int32_t max_args;                        // 最大引数数
    const char* p_description;               // コマンドの説明
} dbg_cmd_info_t;

#define MCU_NAME               "CH32V006F8P6"
#define PCB_NAME               "DEV PCB"
// #define PCB_NAME               "CH32V003F4P6-R0-1V1"
#define MCU_FLASH_SIZE         62
#define MCU_RAM_SIZE           8
#define FW_VERSION_MAJOR       0
#define FW_VERSION_MINOR       0
#define FW_VERSION_REVISION    1

static void dbg_com_init_msg(dbg_cmd_args_t *p_args);

static void cmd_help(dbg_cmd_args_t *p_args);
static void cmd_cls(dbg_cmd_args_t *p_args);
static void cmd_system(dbg_cmd_args_t *p_args);
static void cmd_mem_dump(dbg_cmd_args_t *p_args);
static void cmd_reg(dbg_cmd_args_t *p_args);
// コマンドテーブル
const dbg_cmd_info_t g_cmd_tbl[] = {
//  | コマンド文字列 | コールバック関数 | 最小引数 | 最大引数 | コマンド説明 |
    { "help",         &cmd_help,       0,         0,        "Command All Show"},
    { "cls",          &cmd_cls,        0,         0,        "Display Clear"},
    { "sys",          &cmd_system,     0,         0,        "Show System Information"},
    { "memd",         &cmd_mem_dump,   2,         2,        "Memory Dump Command. args -> (#address, #length)"},
    { "reg",          &cmd_reg,        3,         4,        "Register R/W. exp(reg #addr r|w bits #val)"},
};
#define CMD_TBL_CNT    sizeof(g_cmd_tbl) / sizeof(g_cmd_tbl[0])

static void _cmd_exec(const char *p_buf, dbg_cmd_args_t *p_args);

// コマンドバッファ
static char s_uart_recv_buf[DBG_CMD_UART_BUF_SIZE];
static char s_cmd_buf[DBG_CMD_MAX_LEN];
static uint8_t s_buf_idx = 0;

// -----------------------------------------------------------
// [Static関数]

static void dbg_com_init_msg(dbg_cmd_args_t *p_args)
{
    printf("\nDebug Command Monitor for %s Ver%d.%d.%d\n",MCU_NAME,
                                                        FW_VERSION_MAJOR,
                                                        FW_VERSION_MINOR,
                                                        FW_VERSION_REVISION);
    printf("Copyright (c) 2026 Chimipupu All Rights Reserved.\n");
    printf("Type 'help' for available commands\n");
#ifdef _WDT_ENABLE_
    printf("[INFO] Wanning! WDT Enabled: %dms\n", _WDT_OVF_TIME_MS_);
#endif // _WDT_ENABLE_
}

static void cmd_help(dbg_cmd_args_t *p_args)
{
    dbg_com_init_msg(p_args);

    printf("\nAvailable %d commands:\n", CMD_TBL_CNT);
    for (uint8_t i = 0; i < CMD_TBL_CNT; i++)
    {
        printf("  %-10s - %s\n", g_cmd_tbl[i].p_cmd_str, g_cmd_tbl[i].p_description);
    }
}

static void cmd_cls(dbg_cmd_args_t *p_args)
{
    printf(ANSI_ESC_CLS);
}

static void cmd_system(dbg_cmd_args_t *p_args)
{
    // printf("\n[System Information]\n");

    // 基板
    printf("\n[PCB Info]\nPCB : %s\n", PCB_NAME);

    // マイコン
    printf("MCU : %s\n", MCU_NAME);
    printf("CPU : RISC-V RV32EmC (QingKe V2C)\n");

    // ROM/RAM
    printf("\n[Mem Info]\n");
    printf("Flash Size : %d KB\n", MCU_FLASH_SIZE);
    printf("RAM Size : %d KB\n", MCU_RAM_SIZE);

    // クロック関連
    printf("\n[Clock Info]\n");
    printf("System Clock : %d MHz\r\n", SystemCoreClock / 1000000);
}

static void cmd_mem_dump(dbg_cmd_args_t *p_args)
{
    uint32_t addr;
    uint32_t length;

    if (p_args->argc != 3) {
        printf("Error: Invalid number of arguments. Usage: mem_dump <address> <length>\n");
        return;
    }

    // アドレスを16進数文字列から数値に変換
    if (sscanf(p_args->p_argv[1], "#%x", &addr) != 1) {
        printf("Error: Invalid address format. Use hexadecimal with # prefix (e.g., #F0000000)\n");
        return;
    }

    // 長さを16進数文字列から数値に変換
    if (sscanf(p_args->p_argv[2], "#%x", &length) != 1) {
        printf("Error: Invalid length format. Use hexadecimal with # prefix (e.g., #10)\n");
        return;
    }
}

/**
 * @brief レジスタR/Wコマンド関数
 * @param p_args コマンド引数の構造体ポインタ
 */
static void cmd_reg(dbg_cmd_args_t *p_args)
{
    uint32_t wval = 0;
    uint32_t val = 0;
    uint32_t addr = 0;

    if (p_args->argc != 4 && p_args->argc != 5) {
        printf("Error: Usage: reg #ADDR r|w BITS [#VAL]\n");
        printf("  e.g. reg #F000FF00 r 8\n");
        printf("  e.g. reg #F000FF00 w 32 #FFDC008F\n");
        return;
    }

    if (sscanf(p_args->p_argv[1], "#%x", &addr) != 1) {
        printf("Error: Invalid address format. Use #HEX (e.g. #F000FF00)\n");
        return;
    }
    char rw = p_args->p_argv[2][0];
    int bits = atoi(p_args->p_argv[3]);
    if (!(bits == 8 || bits == 16 || bits == 32)) {
        printf("Error: Bit width must be 8, 16, or 32\n");
        return;
    }
    if (rw == 'r') { // 読み取り
        if (p_args->argc != 4) {
            printf("Error: Read usage: reg #ADDR r BITS\n");
            return;
        }
        if (bits == 8) val = REG_READ_BYTE(0, addr);
        else if (bits == 16) val = REG_READ_WORD(0, addr);
        else if (bits == 32) val = REG_READ_DWORD(0, addr);
        printf("[REG] Read %dbit @ 0x%08X = 0x%08X\n", bits, addr, val);
    } else if (rw == 'w') { // 書き込み
        sscanf(p_args->p_argv[4], "#%x", &wval);
        if (bits == 8) {
            REG_WRITE_BYTE(0, addr, (uint8_t)wval);
        } else if (bits == 16) {
            REG_WRITE_WORD(0, addr, (uint16_t)wval);
        } else if (bits == 32) {
            REG_WRITE_DWORD(0, addr, (uint32_t)wval);
        }
            printf("[REG] Write %dbit @ 0x%08X = 0x%08X\n", bits, addr, wval);
    } else {
            printf("Error: 2nd arg must be 'r' or 'w'\n");
    }
}

static void _cmd_exec(const char *p_buf, dbg_cmd_args_t *p_args)
{
    uint8_t i;
    uint8_t *p_ptr;

    if((p_buf == NULL) || (p_args == NULL)) {
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
        if (strcmp(&s_cmd_buf[0], g_cmd_tbl[i].p_cmd_str) == 0) {
            g_cmd_tbl[i].p_func(p_args);
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
    dbg_cmd_args_t args;
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
            _cmd_exec((const char *)&s_uart_recv_buf[0], &args);
        }

        memset(&s_uart_recv_buf[0], 0x00, DBG_CMD_UART_BUF_SIZE);
        memset(&s_cmd_buf[0], 0x00, DBG_CMD_MAX_LEN);
        s_buf_idx = 0;
        printf("\n> ");
    }
}