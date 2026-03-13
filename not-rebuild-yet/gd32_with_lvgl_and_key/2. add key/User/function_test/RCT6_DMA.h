/**
 * @file RCT6_DMA.h
 * @author Ye Dingzai (yedingzai@126.com)
 * @brief
 * @version 0.1
 * @date 2026-02-12
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef __TEST_RCT6_DMA_H
#define __TEST_RCT6_DMA_H

#include "gd32f30x.h"

void DMA0_Init(void);
void GD32_SPI0_Receive_DMA(uint8_t *rx_buf, uint16_t len);
void DMA0_CHannel1_IRQHandler(void);

#endif

