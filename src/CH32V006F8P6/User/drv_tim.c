/**
 * @file drv_tim.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief  CH32V006 タイマドライバ
 * @version 0.1
 * @date 2026-03-15
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "drv_tim.h"

// -----------------------------------------------------------
typedef struct {
    uint8_t div;
    uint16_t div_config_val;
} drv_tim_div_t;

volatile const drv_tim_div_t g_tim_div_tbl[] = {
    {1, TIM_CKD_DIV1},
    {2, TIM_CKD_DIV2},
    {4, TIM_CKD_DIV4},
};
#define TIM_DIV_TBL_CNT    sizeof(g_tim_div_tbl) / sizeof(g_tim_div_tbl[0])

volatile uint32_t g_systick_cnt_ms = 0;
volatile software_timer_config_t g_sw_timer_buf[SW_TIMER_BUG_SIZE];

static void _sw_timer_init(uint8_t timer_no);
static void _sw_timer_all_init(void);
// -----------------------------------------------------------
// [割り込みハンドラ]

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/**
 * @brief 32bit SysTickタイマ割り込みハンドラ
 * @note 1ms周期で割り込み発生
 */
void SysTick_Handler(void)
{
    // NOTE: 符号なし32bit整数を1ms周期で更新...約49.7日後にOVF
    g_systick_cnt_ms++;

    SysTick->SR = 0; // SysTick割り込みフラグクリア
}

#ifdef USE_TIM_IRQ
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/**
 * @brief TIM1カウントアップ割り込みハンドラ
 */
void TIM1_UP_IRQHandler(void)
{
    ITStatus ret;
    ret = TIM_GetITStatus(TIM1, TIM_IT_Update);
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}
#endif // USE_TIM_IRQ

// -----------------------------------------------------------
// [Static関数]

static void _sw_timer_init(uint8_t timer_no)
{
    if(timer_no <= (SW_TIMER_BUG_SIZE - 1)) {
        g_sw_timer_buf[timer_no].is_cnt_start = false;
        g_sw_timer_buf[timer_no].is_intervel = false;
        g_sw_timer_buf[timer_no].config_time_ms = 0;
        g_sw_timer_buf[timer_no].cnt_time_ms = 0;
    }
}

static void _sw_timer_all_init(void)
{
    uint8_t i;

    for(i = 0; i < SW_TIMER_BUG_SIZE; i++)
    {
        _sw_timer_init(i);
    }
}

// -----------------------------------------------------------
// [ドライバ]
void drv_tick_delay_ms(uint32_t ms)
{
    uint32_t start_tick = drv_get_systick_cnt();
    while ((drv_get_systick_cnt() - start_tick) < ms) {
        __asm__ __volatile__("nop");
    }
}

bool soft_timer_start(uint16_t config_time_ms, bool is_intervel, uint8_t *p_timer_no)
{
    bool ret = false;
    uint8_t i;

    // 空いてるタイマーを探す
    for(i = 0; i < SW_TIMER_BUG_SIZE; i++)
    {
        if(g_sw_timer_buf[i].is_cnt_start != true) {
            g_sw_timer_buf[i].is_intervel = is_intervel;
            g_sw_timer_buf[i].config_time_ms = config_time_ms;
            g_sw_timer_buf[i].cnt_time_ms = 0;
            g_sw_timer_buf[i].is_cnt_start = true;
            *p_timer_no = i;
            ret = true;
            break; // Break for loop
        }
    }

    return ret;
}

void soft_timer_stop(uint8_t timer_no)
{
    if(timer_no <= (SW_TIMER_BUG_SIZE - 1)) {
        g_sw_timer_buf[timer_no].is_cnt_start = false;
    }
}

bool get_soft_timer_cnt_match(uint8_t timer_no)
{
    if(timer_no <= (SW_TIMER_BUG_SIZE - 1)) {
        if(g_sw_timer_buf[timer_no].is_cnt_start != false) {
            // コンペアマッチしてるか？
            if(g_sw_timer_buf[timer_no].cnt_time_ms >= g_sw_timer_buf[timer_no].config_time_ms) {
                // falseのワンショットなら終了、trueの周期タイマーなら再スタート
                if(g_sw_timer_buf[timer_no].is_intervel == false) {
                    g_sw_timer_buf[timer_no].is_cnt_start = false;
                } else {
                    g_sw_timer_buf[timer_no].cnt_time_ms = 0;
                }
                return true;
            } else {
                return false;
            }
        }
    }
}

void soft_timer_proc(void)
{
    uint8_t i;
    static uint32_t s_prev_tick_cnt = 0;
    uint32_t current_tick_cnt;
    uint32_t delta_ms;

    current_tick_cnt = drv_get_systick_cnt();
    delta_ms = current_tick_cnt - s_prev_tick_cnt;
    s_prev_tick_cnt = current_tick_cnt;

    for(i = 0; i < SW_TIMER_BUG_SIZE; i++)
    {
        if(g_sw_timer_buf[i].is_cnt_start != false) {
            g_sw_timer_buf[i].cnt_time_ms += delta_ms; // 1ms加算
        }
    }
}

/**
 * @brief 16bitタイマ  TIM1初期化
 * @param arr 16bit カウント最大値
 * @param psc 16bit プリスケーラ
 * @param div 16bit 分周比(1分周:TIM_CKD_DIV1、2分周:TIM_CKD_DIV2、4分周: TIM_CKD_DIV4)
 */
void drv_tim_init(uint16_t arr, uint16_t psc, uint16_t div)
{
    uint8_t i;
    // NVIC_InitTypeDef NVIC_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    volatile uint16_t div_config_val = TIM_CKD_DIV1; // デフォルトは1分周(TIM_CKD_DIV1)

    // -----------------------------------------------------------
    // [タイマ初期化]
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_TIM1, ENABLE);
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;

    // 分周比の設定値をテーブルから検索(1,2,4分周のいずれかを指定)
    for(i = 0; i < TIM_DIV_TBL_CNT; i++)
    {
        if(g_tim_div_tbl[i].div == div) {
            div_config_val = g_tim_div_tbl[i].div_config_val;
            break;
        }
    }
    TIM_TimeBaseInitStructure.TIM_ClockDivision = div_config_val;

    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 50;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

#ifdef USE_TIM_IRQ
    // -----------------------------------------------------------
    // [タイマ割り込み初期化]
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    // -----------------------------------------------------------
#endif // USE_TIM_IRQ

    TIM_Cmd(TIM1, ENABLE);
}

void drv_systick_init(void)
{
    NVIC_EnableIRQ(SysTick_IRQn);                 // SysTick割り込み有効化
    SysTick->SR &= ~(1 << 0);                     // SysTick割り込みフラグクリア
    SysTick->CMP = (SystemCoreClock - 1) / 1000;  // SysTick割り込み = 1ms周期
    SysTick->CNT = 0;                             // SysTickカウント値をクリア
    SysTick->CTLR = 0xF;

    // S/Wタイマーカウント初期化
    _sw_timer_all_init();
}