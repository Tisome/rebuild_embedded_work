#ifndef __UART_PARSE_TASK_H__
#define __UART_PARSE_TASK_H__

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h" 
#include "queue.h"
#include "elog.h"
#include "mid_circular_buffer.h"

#include "bsp_uart_driver.h"



void uart_rec_A_func(void *argument);

uint8_t buffer1[1] = {0};
uint8_t buffer2[2] = {0};





#endif // end __UART_PARSE_TASK_H__
