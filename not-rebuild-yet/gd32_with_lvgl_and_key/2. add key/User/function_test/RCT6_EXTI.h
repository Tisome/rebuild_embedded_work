/**
 * @file RCT6_EXTI.h
 * @author Ye Dingzai (yedingzai@126.com)
 * @brief
 * @version 0.1
 * @date 2026-02-12
 *
 * @copyright Copyright (c) 2026
 * @note 这里设定了RCT6系统核心板上的SW1按键的宏定义
 *
 */

#ifndef __TEST_RCT6_EXTI_H
#define __TEST_RCT6_EXTI_H

void KEY_SW1_EXTI_Init(void);
void FPGA_INT_EXTI_Init(void);
void EXTI5_9_IRQHandler(void);
void EXTI1_IRQHandler(void);

#endif
