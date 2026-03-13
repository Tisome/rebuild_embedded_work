#ifndef __USART_H__
#define __USART_H__
#include "stdint.h"

void usart0_dma_init(uint8_t* rxbuffer,uint32_t buflen);

void usart0_send_bytes(uint8_t *ch,uint32_t len);

#endif