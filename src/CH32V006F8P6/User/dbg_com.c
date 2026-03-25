/**
 * @file dbg_com.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief デバッグUARTモニタ
 * @version 0.1
 * @date 2026-03-26
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#include "dbg_com.h"
#include "ansi_esc.h"

// -----------------------------------------------------------
// [DEBUG関連]

// -----------------------------------------------------------
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
//  | 文字列 | 種類 | コールバック関数 | 最小引数 | 最大引数 | 説明 |
    {"help",    CMD_HELP,       &cmd_help,        0,    0,    "Command All Show"},
    {"cls",     CMD_CLS,        &cmd_cls,         0,    0,    "Display Clear"},
    {"sys",     CMD_SYSTEM,     &cmd_system,      0,    0,    "Show System Information"},
    {"memd",    CMD_MEM_DUMP,   &cmd_mem_dump,    2,    2,    "Memory Dump Command. args -> (#address, #length)"},
    {"reg",     CMD_REG,        &cmd_reg,         3,    4,    "Register R/W. exp(reg #addr r|w bits #val)"},
};
const size_t g_cmd_tbl_size = sizeof(g_cmd_tbl) / sizeof(g_cmd_tbl[0]);

static void _cmd_exec(dbg_cmd_t cmd, dbg_cmd_args_t *p_args);

// コマンドバッファ
static char s_cmd_buffer[DBG_CMD_MAX_LEN];
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

    printf("\nAvailable %d commands:\n", g_cmd_tbl_size);
    for (uint8_t i = 0; i < g_cmd_tbl_size; i++)
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

// コマンド引数を分割して解析
static int32_t _cmd_split(char* p_str, dbg_cmd_args_t *p_args)
{
    char* p_token;
    char* p_next = p_str;
    char* p_end = p_str + strlen(p_str);

    p_args->argc = 0;
#ifdef DEBUG_DBG_COM
    printf("[DEBUG] : Input string = '%s'\n", p_str);  // 入力文字列の確認
#endif // DEBUG_DBG_COM

    while (p_next < p_end && p_args->argc < DBG_CMD_MAX_ARGS)
    {
        // スペースをスキップ
        while (*p_next == ' ' && p_next < p_end)
        {
            p_next++;
        }
        if (p_next >= p_end) break;

        // トークンの開始位置を記録
        p_token = p_next;

        // 次のスペースまたは文字列末尾まで移動
        while (*p_next != ' ' && p_next < p_end)
        {
            p_next++;
        }

        // 文字列を終端
        if (*p_next == ' ') {
            *p_next++ = '\0';
        }

#ifdef DEBUG_DBG_COM
        printf("[DEBUG] : Next token = '%s'\n", p_token);
#endif // DEBUG_DBG_COM
        p_args->p_argv[p_args->argc++] = p_token;
        // WDT_RST();
    }

#ifdef DEBUG_DBG_COM
    printf("[DEBUG] : argc = %d\n", p_args->argc);
    for (int i = 0; i < p_args->argc; i++)
    {
        printf("[DEBUG] : argv[%d] = %s\n", i, p_args->p_argv[i]);
    }
#endif // DEBUG_DBG_COM

    return p_args->argc;
}

/**
 * @brief コマンド文字列を解析してコマンド種類を返す
 * @param p_cmd_str コマンド文字列
 * @param p_args 引数構造体
 * @return dbg_cmd_t コマンド種類
 */
static dbg_cmd_t _cmd_parse(const char* p_cmd_str, dbg_cmd_args_t *p_args)
{
    for (uint8_t i = 0; i < g_cmd_tbl_size; i++)
    {
        if (strcmp(p_cmd_str, g_cmd_tbl[i].p_cmd_str) == 0) {
            // 引数の数をチェック
            // if (p_args->argc - 1 < g_cmd_tbl[i].min_args || p_args->argc - 1 > g_cmd_tbl[i].max_args) {
            //     printf("Error: Invalid number of arguments. Expected %d-%d, got %d\n",
            //         g_cmd_tbl[i].min_args, g_cmd_tbl[i].max_args, p_args->argc - 1);
            //     return CMD_UNKNOWN;
            // }
            return g_cmd_tbl[i].cmd_type;
        }
    }
    return CMD_UNKNOWN;
}

/**
 * @brief コマンドを実行する
 * @param cmd コマンド種類
 * @param p_args 引数構造体
 */
static void _cmd_exec(dbg_cmd_t cmd, dbg_cmd_args_t *p_args)
{
    uint8_t i;

    for(i = 0; i < g_cmd_tbl_size; i++)
    {
        if(g_cmd_tbl[i].cmd_type == cmd) {
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
    dbg_cmd_t cmd;
    bool drv_ret;

    // UARTから1バイト受信
    drv_ret = drv_uart_get_char(&c);
    if(drv_ret == false) {
        return;
    }

    // リングバッファ更新
    s_cmd_buffer[s_buf_idx] = c;
    s_buf_idx = (s_buf_idx + 1) % DBG_CMD_MAX_LEN;

    // デリミタでCRかLFが来てたらコマンドを受付ける
    if (c == '\r' || c == '\n') {
        if (s_buf_idx > 0) {
            s_cmd_buffer[s_buf_idx] = '\0';
            printf("\n");

            // コマンド解析
            _cmd_split(s_cmd_buffer, &args);
            cmd = _cmd_parse(args.p_argv[0], &args);

            // コマンド実行
            _cmd_exec(cmd, &args);

            memset(&s_cmd_buffer[0], 0x00, DBG_CMD_MAX_LEN);
            s_buf_idx = 0;
        }

        printf("\n> ");
    }
}