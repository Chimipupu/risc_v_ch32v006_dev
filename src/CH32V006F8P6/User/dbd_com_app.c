/**
 * @file dbd_com_app.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief デバッグモニタのアプリ
 * @version 0.1
 * @date 2025-08-02
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "dbg_com.h"
#include "ansi_esc.h"

#define MCU_NAME               "CH32V006F8P6"
#define PCB_NAME               "DEV PCB"
#define MCU_FLASH_SIZE         64
#define MCU_RAM_SIZE           8
#define FW_VERSION_MAJOR       0
#define FW_VERSION_MINOR       0
#define FW_VERSION_REVISION    1

static void dbg_com_init_msg(dbg_cmd_args_t *p_args);

void cmd_help(dbg_cmd_args_t *p_args);
static void cmd_cls(dbg_cmd_args_t *p_args);
static void cmd_system(dbg_cmd_args_t *p_args);

// コマンドテーブル
const dbg_cmd_info_t g_cmd_tbl[] = {
//  | 文字列 | 種類 | コールバック関数 | 最小引数 | 最大引数 | 説明 |
    {"help",    CMD_HELP,       &cmd_help,        0,    0,    "Command All Show"},
    {"cls",     CMD_CLS,        &cmd_cls,         0,    0,    "Display Clear"},
    {"sys",     CMD_SYSTEM,     &cmd_system,      0,    0,    "Show System Information"},
};

// コマンドテーブルのコマンド数(const)
const size_t g_cmd_tbl_size = sizeof(g_cmd_tbl) / sizeof(g_cmd_tbl[0]);

static void dbg_com_init_msg(dbg_cmd_args_t *p_args)
{
    printf("\nDebug Command Monitor for %s Ver%d.%d.%d\n",MCU_NAME,
                                                        FW_VERSION_MAJOR,
                                                        FW_VERSION_MINOR,
                                                        FW_VERSION_REVISION);
    printf("Copyright (c) 2025 Chimipupu All Rights Reserved.\n");
    printf("Type 'help' for available commands\n");
#ifdef _WDT_ENABLE_
    printf("[INFO] Wanning! WDT Enabled: %dms\n", _WDT_OVF_TIME_MS_);
#endif // _WDT_ENABLE_
}

void cmd_help(dbg_cmd_args_t *p_args)
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
    printf("\n[System Information]\n");

    // 基板
    printf("\n[PCB Info]\nPCB Name : %s\n", PCB_NAME);

    // マイコン
    printf("MCU : %s\n", MCU_NAME);
    printf("CPU : RISC-V  RV32EC (QingKe V2)\n");

    // ROM/RAM
    printf("\n[Mem Info]\n");
    printf("Flash Size : %d KB\n", MCU_FLASH_SIZE);
    printf("RAM Size : %d KB\n", MCU_RAM_SIZE);

    // クロック関連
    printf("\n[Clock Info]\n");
    printf("SystemClk:%d\r\n", SystemCoreClock);
}