#include "app_config.h"

#if (RCT6 || CGT6)

#ifndef __GD32F303_DMA_H
#define __GD32F303_DMA_H

#include "gd32f30x.h"

void DMA0_Init(void);
void GD32_SPI0_Receive_DMA(uint8_t *rx_buf, uint16_t len);
void DMA0_CHannel1_IRQHandler(void);

#endif

#endif
