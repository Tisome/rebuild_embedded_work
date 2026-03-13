/*
**********************************************************************************
*                                 RUF-X??
**********************************************************************************
*/

#ifndef __STM32_RUF_X_DEMO_H
#define __STM32_RUF_X_DEMO_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

extern uint16_t sKeyFlag; // 按键滤波标志位

extern SemaphoreHandle_t xSem_key_Filter;  // 按键滤波信号量句柄
extern SemaphoreHandle_t xSem_FPGA_INT;    // FPGA中断信号量句柄
extern SemaphoreHandle_t xSem_SPI_Rx_Done; // SPI接收完成信号量句柄

extern QueueHandle_t xQueue_AlgoOut;      // 算法输出队列句柄
extern QueueHandle_t xQueue_Rx_Index_Buf; // SPI接收数据队列句柄

void vSemaphoreInit(void); // 信号量初始化函数
void vQueueInit(void);     // 队列初始化函数

void RUF_X_demo(void);

#endif
