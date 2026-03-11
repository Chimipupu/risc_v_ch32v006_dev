/**
 * @file pcb_board.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 基板固有の定義ヘッダ
 * @version 0.1
 * @date 2026-03-09
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef PCB_BOARD_H
#define PCB_BOARD_H

// ----------------------------------------------------------------------
// [コンパイルスイッチ]
#define DEBUG_UART_USE // UARTの使用有無
#define DEBUG_I2C_USE // I2Cの使用有無

#ifdef DEBUG_I2C_USE
    #define EEPROM_USE
#endif // DEBUG_I2C_USE
// ----------------------------------------------------------------------

#endif // PCB_BOARD_H