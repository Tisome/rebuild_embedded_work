#include "app_config.h"

#if (RCT6 || CGT6)

#ifndef __GD32F303_USART_H
#define __GD32F303_USART_H

#include "gd32f30x.h"

void USART0_Init(void);
void USART0_SendChar(uint8_t c);

#endif

#endif


