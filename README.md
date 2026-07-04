# CH32V006 評価F/W 個人開発

WCH製25円 RISC-Vマイコン CH32V006の評価F/W個人開発リポジトリ

## 開発環境

- S/W
  - IDE/SDK/コンパイラ
    - [MounRiver Studio (MRS) V2.5.0](https://www.mounriver.com/download)🔗
  - 最適化
    - `-O0`
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
- デバッグ
  - [WCH-LinkE Ver1.3](https://akizukidenshi.com/catalog/g/g118065)🔗

## メモリ使用量

- 最適化
  - `-O0`
- EEPROM
  - **あり**

```shell
Memory region         Used Size  Region Size  %age Used
           FLASH:       10560 B        62 KB     16.63%
             RAM:         820 B         8 KB     10.01%
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

## 評価総評

- 総評
  - ★★★★☆ 90/100点

- 良い点
  - 単価25円で破格すぎるRISC-Vマイコン
  - 5Vでも3.3Vでも動かせる貴重な32bitのマイコン
  - CPUがRV32EmCで乗算（かけ算）をCPU命令のハードウェア
  - ROMとRAMが単価を考えてもかなり多い
  - I2CとSPIは1本、UARTは2本、16bitタイマーは3本
  - DMAが7本も使える
  - ADCが12bitで8本も使える
  - デバッグが1本のシリアルで出来る
  - マイコンのチップ毎にUIDが96bitも付与されてる
  - 内蔵の24MHz HSIが誤差2%
- 悪い点
  - 内蔵フラッシュROMで2ウェイトも発生する（48MHz時）
  - PLLが2逓倍固定で2倍にしかクロックの周波数を上げれない
  - DeepSleepが10μAでESP32の約2倍
  - WCHからSDKの更新がない
  - 除算（割り算）がソフトウェア
