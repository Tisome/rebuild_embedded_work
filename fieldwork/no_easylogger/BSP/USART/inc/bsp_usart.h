#ifndef __BSP_USART_H
#define __BSP_USART_H

#define IRQ_SEND_TO_UART_DRIVER_TASK  0xA1A2A3A4
#define UART_TASK_SEND_TO_MODBUS_TASK 0xB1B2B3B4

#include "usart.h"

void task_uart_driver(void *parameter);

void uart1_idle_irq_callback(uint16_t pos);

void uart1_dma_half_irq_callback(void);

void uart1_dma_full_irq_callback(void);

#endif