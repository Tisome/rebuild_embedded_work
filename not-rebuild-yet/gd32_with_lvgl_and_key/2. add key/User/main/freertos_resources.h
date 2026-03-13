#ifndef __FREERTOS_RESOURCES_H
#define __FREERTOS_RESOURCES_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#if (SYS_TEST == 1)
extern SemaphoreHandle_t xSem_ISR;    // 按键SW1测试信号量句柄
#endif

extern SemaphoreHandle_t xSem_FPGA_INT;    // FPGA中断信号量句柄
extern SemaphoreHandle_t xSem_SPI_Rx_Done; // SPI接收完成信号量句柄

extern QueueHandle_t xQueue_AlgoOut;      // 算法输出队列句柄
extern QueueHandle_t xQueue_Rx_Index_Buf; // SPI接收数据队列句柄
extern QueueHandle_t xQueue_KeyEvent;     // 按键接收队列句柄
#endif
