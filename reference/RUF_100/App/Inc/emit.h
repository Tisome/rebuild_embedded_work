#ifndef __EMIT_H__
#define __EMIT_H__
#include "gd32f30x.h"

// 扫频参数配置
#define FREQ_COUNT       5               // 扫频频率点数量
#define PULSE_PER_FREQ   15              // 每个频率输出的脉冲数       // 扫频运行标志（1：运行中，0：停止）

void emit_init(void);
void sweep_start(void);
void pwm_set_freq(uint32_t freq);
void loop_switch(void);
#endif