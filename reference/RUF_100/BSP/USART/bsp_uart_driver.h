#ifndef __BSP_UART_DRIVER_H__
#define __BSP_UART_DRIVER_H__


#include "usart.h"

#include "FreeRTOS.h"
#include "task.h" 
#include "queue.h"

#include "mid_circular_buffer.h"
#include "elog.h"


void uart_driver_func(void *argument);


circular_buffer_t * get_circular_buffer(void);


void dma_half_irq_callback(uint32_t number_of_data);

void dma_comp_irq_callback(uint32_t number_of_data);

void uart_idle_irq_callback(uint32_t number_of_data);



#endif // end __BSP_UART_DRIVER_H__
