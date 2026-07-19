/**
 * @file app_mem.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリ用メモリ
 * @version 0.1
 * @date 2026-07-05
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_mem.h"

#ifdef EEPROM_USE
// -----------------------------------------------------------

// -----------------------------------------------------------
// [API]

bool app_mem_e2p_factory_reset(void)
{
    bool ret = true;
#if 0
    int ret_cmp;
    uint8_t e2p_page_buf[EEPROM_24C64_PAGE_BYTE_SIZE] = {0};

    drv_eeprom_read_page(0x00, (uint8_t *)&e2p_page_buf[0]);
    ret_cmp = memcmp((const void *)&g_test_ascii_tbl,
                     (const void *)&e2p_page_buf,
                        EEPROM_24C64_PAGE_BYTE_SIZE);

    // EEPROMを工場出荷リセット
    if(ret_cmp != 0) {
        ret = true;
        drv_eeprom_write_page(0x00, (uint8_t *)&g_test_ascii_tbl[0]);
        drv_tick_delay_ms(10); // EEPROMの書き込み待ち時間の8ms以上待つ
        drv_eeprom_read_page(0x00, (uint8_t *)&e2p_page_buf[0]);
        DEBUG_PRINTF("[DEBUG] EEPROM Factory Reset Done!\r\n");
    } else {
        ret = false;
        DEBUG_PRINTF("[DEBUG] This EEPROM Aleady Factory Reseted!\r\n");
    }

    // EEPORM メモリダンプ
    app_util_mem_dump((const uint8_t *)&e2p_page_buf[0], EEPROM_24C64_PAGE_BYTE_SIZE);

#endif
    return ret;
}

void app_mem_get_e2p_data(uint32_t e2p_data_id, void *p_read_buf)
{
    // TODO
}

void app_mem_set_e2p_data(uint32_t e2p_data_id, void *p_write_buf)
{
    // TODO
}
#endif