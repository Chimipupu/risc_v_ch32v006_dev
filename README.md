# CH32V006 評価F/W 個人開発

WCH製25円 RISC-Vマイコン CH32V006の評価F/W個人開発リポジトリ

## 開発環境

- S/W
  - IDE/SDK/コンパイラ
    - [MounRiver Studio (MRS) V2.5.0](https://www.mounriver.com/download)🔗
  - 最適化
    - `-Os` (サイズ優先)
  - コーディング規約
    - https://github.com/Chimipupu/c_lang_coding_conventions_for_chimipupu.git
- H/W
  - マイコン
    - [CH32V006F8P6](https://www.wch-ic.com/downloads/CH32V006DS0_PDF.html)🔗
      - CPU
        - `QingKeV2C` (RV32EmC)
          - 乗算 ... H/W (CPU命令)
          - 除算 ... S/W
      - ROM ... 62KB
      - RAM ... 8KB
      - Clock ... 48MHz
      - GPIO ... 18本
      - DMA ... x7ch
      - タイマー
        - TIM1 ... 16bit 高機能タイマー
        - TIM2 ... 16bit 汎用タイマー
        - TIM3 ... 16bit 基本タイマー
      - WDT ... x2本(IWDG, WWDG)
      - SysTick ... 32bitタイマー
      - I2C ... x1ch
      - SPI ... x1ch
      - USART ... x2ch
      - ADC ... 12bit 3Msps SAR x8ch
      - OPA ... x3ch
- デバッガ
  - [WCH-LinkE Ver1.3](https://akizukidenshi.com/catalog/g/g118065)🔗

## コンパイルスイッチ

```c
// -----------------------------------------------------------
// [コンパイルスイッチ]
#define DEBUG_UART_USE // UARTの使用有無
// #define DEBUG_I2C_USE  // I2Cの使用有無

// #define USE_BUTTON  // 基板のボタン使用有無
// #define USE_74HC595  // 74HC595の使用有無

// #define EEPROM_USE     // EEPROMの使用有無
#if !defined(DEBUG_I2C_USE) && defined(EEPROM_USE)
#error "[ERROR] Please define -> DEBUG_I2C_USE at pcb_board.h!"
#endif

#define I2C_RTC_DEVICE                I2C_ENV_SENSOR_NONE
// #define I2C_RTC_DEVICE                I2C_RTC_DS3231
// #define I2C_RTC_DEVICE                I2C_RTC_RX8900

#define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_NONE
// #define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_AHT20
// #define I2C_ENV_SENSOR_DEVICE         I2C_ENV_SENSOR_BMP280
// -----------------------------------------------------------
```

## メモリ使用量

- 最適化
  - `-Os` (サイズ優先)
- 注記
  - 上記、コンパイルスイッチでF/Wを`-Os`でビルド

```shell
Memory region         Used Size  Region Size  %age Used
           FLASH:        5660 B        62 KB      8.92%
             RAM:         832 B         8 KB     10.16%
```

## ピンアサイン

<div align="center">
  <img src="/doc/CH32V006_pinout.png">
</div>

- SDI
  - WCHの1線式デバッグ

| WCH-LinkE | CH32V006F8P6 |
|-|-|
| SWDIO | PD1|
| GND | GND |

### UART

| WCH-LinkE | CH32V006F8P6 |
|-|-|
| TX | PD6 (RX)|
| RX | PD5 (TX)|
| GND | GND |

### I2C

| I2C | CH32V006F8P6 |
|-|-|
| SDA | PC1 (SDA)|
| SCL | PC2 (SCL)|

### SPI

| SPI | CH32V006F8P6 |
|-|-|
| CS   | 任意のGPIO  |
| SCK  | PC5 (SCK)  |
| MOSI | PC6 (MOSI) |
| MISO | PC7 (MISO) |

