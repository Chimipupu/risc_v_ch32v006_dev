/**
 * @file app_mem.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリ用メモリ
 * @version 0.1
 * @date 2026-07-05
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef APP_MEM_H
#define APP_MEM_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "pcb_board_define.h"
#include "app_io_reg.h"

#ifdef EEPROM_USE
#include "drv_i2c_eeprom_24c64.h"
void app_mem_get_e2p_data(uint32_t e2p_data_id, void *p_read_buf);
void app_mem_set_e2p_data(uint32_t e2p_data_id, void *p_write_buf);
#endif // EEPROM_USE

#endif // APP_MEM_H