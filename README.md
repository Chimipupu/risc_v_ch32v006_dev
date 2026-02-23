# RISC-Vマイコン CH32V006 評価F/W開発
WCH製 RISC-Vマイコン CH32V006の評価F/W個人開発リポジトリ

## 開発環境

### S/W

- IDE/SDK/コンパイラ
  - [MounRiver Studio (MRS) V2.2.0](https://www.mounriver.com/download)🔗

### H/W

<!-- - 評価基板
  - `CH32V00xEVT(CH32V003F4P6-R0-1V1)`
  - ※基板からC`H32V003F4P6`を剥がして`CH32V006F8P6`に貼り替え済み -->

- マイコン ... 型番:[CH32V006F8P6](https://www.wch-ic.com/downloads/CH32V006DS0_PDF.html)🔗
  - CPU ... [QingKeV2C (32bit RV32EmC RISC-V)]
  - ROM ... 62KB
  - RAM ... 8KB
  - Clock ... 48MHz
  - GPIO ... 18本
  - DMA ... x7ch
  - タイマー
    - TIM1 ... 16bit高機能タイマー
    - TIM2 ... 16bit汎用タイマー
    - TIM3 ... 16bit汎用タイマー
  - WDT ... x2本(IWDG, WWDG)
  - SysTick ... 32bitタイマー
  - I2C ... x1ch
  - SPI ... x1ch
  - USART ... x2ch
  - ADC ... 12bit 3Msps SAR x8ch
  - OPA ... x3ch

<div align="center">
  <img src="/doc/CH32V006_pinout.png">
</div>


#### デバッガ

- [WCH-LinkE Ver1.3](https://akizukidenshi.com/catalog/g/g118065)🔗

#### デバッグ()SDI

- SDI ... WCHの1線式デバッグ

| WCH-LinkE | CH32V006F8P6 |
|-|-|
| SWDIO | PD1|
| GND | GND |

#### UART

| WCH-LinkE | CH32V006F8P6 |
|-|-|
| TX | PD6 (RX)|
| RX | PD5 (TX)|
| GND | GND |

#### I2C

| I2C | CH32V006F8P6 |
|-|-|
| SDA | PC1 (SDA)|
| SCL | PC2 (SCL)|

#### SPI

| SPI | CH32V006F8P6 |
|-|-|
| CS   | 任意のGPIO  |
| SCK  | PC5 (SCK)  |
| MOSI | PC6 (MOSI) |
| MISO | PC7 (MISO) |
