/*
**********************************************************************************
*                                 RUF-X例程
**********************************************************************************
*/

/*********************************GD32 SPL库*************************************/
#include "gd32f30x.h"

/*******************************线程和移植文件*************************************/

#include "RUF_X_demo.h"
#include "algorithmTask.h"
#include "SPI_RxTask.h"
#include "show_UI.h"
#include "fakeFPGATask.h"
#include "watchdogTask.h"

#include "exti.h"
#include "spi.h"
#include "dma.h"
#include "usart.h"

#include "key.h"
#include "FPGA_INT.h"

#if RCT6
#include "led.h"
#endif

#if SYS_TEST
#include <stdio.h>
#endif

#include "app_config.h"
#include "irq_config.h"

/*****************************FreeRTOS INCLUDE*************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/*****************************FreeRTOS 任务配置*************************************/

// SPI数据接收任务配置
#define SPI_RECEIVER_TASK_PRIO 10             // 任务优先级
#define SPI_RECEIVER_STK_SIZE 160             // 任务堆栈大小
#define SPI_RECEIVER_TASK_NAME "SPI_Receiver" // 任务名称
TaskHandle_t xSPI_RxTask_Handler;             // 任务句柄
void vSPI_Rx_task(void *pvParameter);         // 任务函数

// 数据处理任务配置
#define ALGORITHM_TASK_PRIO 9            // 任务优先级
#define ALGORITHM_STK_SIZE 256           // 任务堆栈大小
#define ALGORITHM_TASK_NAME "Algorithm"  // 任务名称
TaskHandle_t xAlgorithmTask_Handler;     // 任务句柄
void vAlgorithm_task(void *pvParameter); // 任务函数

// 显示数据任务配置
#define SHOW_UI_TASK_PRIO 4            // 任务优先级
#define SHOW_UI_STK_SIZE 640          // 任务堆栈大小
#define SHOW_UI_TASK_NAME "ShowUI"     // 任务名称
TaskHandle_t xShowUITask_Handler;      // 任务句柄
void vShow_UI_task(void *pvParameter); // 任务函数

#define WATCHDOG_TASK_PRIO 3            // 任务优先级
#define WATCHDOG_STK_SIZE 128           // 任务堆栈大小
#define WATCHDOG_TASK_NAME "Watchdog"   // 任务名称
TaskHandle_t xWatchdogTask_Handler;     // 任务句柄
void vWatchdog_task(void *pvParameter); // 任务函数

// START_TASK任务配置
#define START_TASK_PRIO 1            // 任务优先级
#define START_STK_SIZE 96            // 任务堆栈大小
#define START_TASK_NAME "START_TASK" // 任务名称
TaskHandle_t xStartTask_Handler;     // 任务句柄
void start_task(void *pvParameter);  // 任务函数

#if USE_FAKE_FPGA

// 测试功能的任务，直接给algo发包看看能不能处理数据，之后会在show_UI中显示情况
#define FAKE_FPGA_TASK_PRIO 8           // 任务优先级
#define FAKE_FPGA_STK_SIZE 192          // 任务堆栈大小
#define FAKE_FPGA_TASK_NAME "FakeFPGA"  // 任务名称
TaskHandle_t xFakeFPGA_Task_Handler;    // 任务句柄
void vFakeFPGA_task(void *pvParameter); // 任务函数

#endif

#if USE_KEY_SW1

// 在其中实现按键任务，RCT6上专用，任务函数在exti中
#define KEY_TASK_PRIO 7            // 任务优先级
#define KEY_STK_SIZE 256           // 任务堆栈大小
#define KEY_TASK_NAME "FakeFPGA"   // 任务名称
TaskHandle_t xkey_Task_Handler;    // 任务句柄
void vkey_task(void *pvParameter); // 任务函数

#endif

/***********************************************************************************/
// 会用到的全局状态变量、信号量、队列等资源的句柄声明放在这里

#if USE_KEY_SW1
SemaphoreHandle_t xSem_ISR = NULL; // 按键SW1测试信号量句柄
#endif

SemaphoreHandle_t xSem_FPGA_INT = NULL;    // FPGA中断信号量句柄
SemaphoreHandle_t xSem_SPI_Rx_Done = NULL; // SPI数据已接收信号量句柄
QueueHandle_t xQueue_Rx_Index_Buf = NULL;  // 接收数据队列句柄
QueueHandle_t xQueue_AlgoOut = NULL;       // 算法输出队列句柄
QueueHandle_t xQueue_KeyEvent = NULL;      // 按键接收队列句柄
void vSemaphoreInit(void);                 // 信号量初始化函数
void vQueueInit(void);                     // 队列初始化函数
void vPeripheralsInit(void);               // 各个外设的初始化
/***********************************************************************************/

void RUF_X_demo(void)
{
    // 信号量初始化
    vSemaphoreInit();

    // 队列初始化
    vQueueInit();

    // 各个外设初始化
    vPeripheralsInit();

#if SYS_TEST
    printf("RCT6 RUF TESTING!");
#else
    printf("CGT6 RUF TESTING!");
#endif

    // 创建开始任务
    xTaskCreate((TaskFunction_t)start_task,           // 任务函数
                (const char *)START_TASK_NAME,        // 任务名称
                (uint16_t)START_STK_SIZE,             // 任务堆栈大小
                (void *)NULL,                         // 任务传入参数
                (UBaseType_t)START_TASK_PRIO,         // 任务优先级
                (TaskHandle_t *)&xStartTask_Handler); // 任务句柄
    vTaskStartScheduler();
}

#if SYS_TEST
void vPrintTaskStackInfo(void)
{
    printf("[ui] init ok, heap free=%lu, min=%lu, stack_hwm=%lu\r\n",
           (unsigned long)xPortGetFreeHeapSize(),
           (unsigned long)xPortGetMinimumEverFreeHeapSize(),
           (unsigned long)uxTaskGetStackHighWaterMark(NULL));
    printf("[stack] HWM(words): START=%lu SPI=%lu ALG=%lu UI=%lu",
           (unsigned long)uxTaskGetStackHighWaterMark(xStartTask_Handler),
           (unsigned long)uxTaskGetStackHighWaterMark(xSPI_RxTask_Handler),
           (unsigned long)uxTaskGetStackHighWaterMark(xAlgorithmTask_Handler),
           (unsigned long)uxTaskGetStackHighWaterMark(xShowUITask_Handler));

#if USE_FAKE_FPGA
    printf(" FAKE=%lu",
           (unsigned long)uxTaskGetStackHighWaterMark(xFakeFPGA_Task_Handler));
#endif

#if USE_KEY_SW1
    if (xkey_Task_Handler != NULL)
    {
        printf(" KEY=%lu",
               (unsigned long)uxTaskGetStackHighWaterMark(xkey_Task_Handler));
    }
#endif

    printf("\r\n");

    printf("[heap] free=%lu min=%lu\r\n",
           (unsigned long)xPortGetFreeHeapSize(),
           (unsigned long)xPortGetMinimumEverFreeHeapSize());
}
#endif

void start_task(void *pvParameters)
{
    (void)pvParameters;

    taskENTER_CRITICAL();

    // 创建SPI数据接收任务
    xTaskCreate((TaskFunction_t)vSPI_Rx_task,          // 任务函数
                (const char *)SPI_RECEIVER_TASK_NAME,  // 任务名称
                (uint16_t)SPI_RECEIVER_STK_SIZE,       // 任务堆栈大小
                (void *)NULL,                          // 任务传入参数
                (UBaseType_t)SPI_RECEIVER_TASK_PRIO,   // 任务优先级
                (TaskHandle_t *)&xSPI_RxTask_Handler); // 任务句柄

    // 创建数据处理任务
    xTaskCreate((TaskFunction_t)vAlgorithm_task,          // 任务函数
                (const char *)ALGORITHM_TASK_NAME,        // 任务名称
                (uint16_t)ALGORITHM_STK_SIZE,             // 任务堆栈大小
                (void *)NULL,                             // 任务传入参数
                (UBaseType_t)ALGORITHM_TASK_PRIO,         // 任务优先级
                (TaskHandle_t *)&xAlgorithmTask_Handler); // 任务句柄

    // 创建显示数据任务
    xTaskCreate((TaskFunction_t)vShow_UI_task,
                (const char *)SHOW_UI_TASK_NAME,
                (uint16_t)SHOW_UI_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)SHOW_UI_TASK_PRIO,
                (TaskHandle_t *)&xShowUITask_Handler);
    // 创建watchdog任务
    xTaskCreate((TaskFunction_t)vWatchdog_task,
                (const char *)WATCHDOG_TASK_NAME,
                (uint16_t)WATCHDOG_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)WATCHDOG_TASK_PRIO,
                (TaskHandle_t *)&xWatchdogTask_Handler);

#if USE_FAKE_FPGA
    // 创建FakeFPGA任务
    {
        BaseType_t ret = xTaskCreate((TaskFunction_t)vFakeFPGA_task,           // 任务函数
                                     (const char *)FAKE_FPGA_TASK_NAME,        // 任务名称
                                     (uint16_t)FAKE_FPGA_STK_SIZE,             // 任务堆栈大小
                                     (void *)NULL,                             // 任务传入参数
                                     (UBaseType_t)FAKE_FPGA_TASK_PRIO,         // 任务优先级
                                     (TaskHandle_t *)&xFakeFPGA_Task_Handler); // 任务句柄
#if SYS_TEST
        printf("create FakeFPGA ret=%ld\r\n", (long)ret);
#endif
    }
#endif

    taskEXIT_CRITICAL();

    vPrintTaskStackInfo();

    vTaskDelete(NULL); /* 删除开始任务 */
}
void vSemaphoreInit(void)
{
#if USE_KEY_SW1
    // 创建SW1模拟的 FPGA中断信号量
    xSem_ISR = xSemaphoreCreateBinary();
    configASSERT(xSem_ISR != NULL);
#endif

    // 创建FPGA中断信号量
    xSem_FPGA_INT = xSemaphoreCreateBinary();
    configASSERT(xSem_FPGA_INT != NULL);

    // 创建SPI数据已接收信号量
    xSem_SPI_Rx_Done = xSemaphoreCreateBinary();
    configASSERT(xSem_SPI_Rx_Done != NULL);
}

void vQueueInit(void)
{
    // 创建接收数据队列
    xQueue_Rx_Index_Buf = xQueueCreate(10, sizeof(rufx_raw_packet_t));
    configASSERT(xQueue_Rx_Index_Buf != NULL);

    // 创建算法输出队列
    xQueue_AlgoOut = xQueueCreate(1, sizeof(algo_out_t));
    configASSERT(xQueue_AlgoOut != NULL);

    xQueue_KeyEvent = xQueueCreate(10, sizeof(key_event_t));
    configASSERT(xQueue_KeyEvent != NULL);
}

void vPeripheralsInit(void)
{
    NVIC_SetPriorityGrouping(0);
    // 各个外设初始化
    USART0_Init();
    SPI0_Init();
    DMA0_Init();
    EXTI_Init();
#if RCT6
    LED_Init();
#endif
    IRQ_Config();

    printf("DMA prio=%lu, ", (unsigned long)NVIC_GetPriority(DMA0_Channel1_IRQn));
#if USE_KEY_SW1
    printf("SW1 KEY prio=%lu, ", (unsigned long)NVIC_GetPriority(SW1_KEY_EXTI_IRQn));
#endif
    printf("FPGA prio=%lu\r\n", (unsigned long)NVIC_GetPriority(FPGA_INT_GPIO_IRQn));
}
