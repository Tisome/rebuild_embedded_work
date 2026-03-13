/*
**********************************************************************************
*                                 RUF-X例程
**********************************************************************************
*/

#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"

/*******************************线程和移植文件*************************************/

#include "./Thread/DEMO/RUF_X_demo.h"
#include "./Thread/ALGO/algorithmTask.h"
#include "./Thread/SPI_RX/SPI_RxTask.h"
#include "./Thread/KEY_FILTER/keyFilterTask.h"
#include "./Thread/SHOW_UI/show_UI.h"
#include "./Thread/FakeFPGA/fakeFPGATask.h"

#include "./CONFIG/app_config.h"

#include "./MCU_Port/EXTI/exti.h"
#include "./MCU_Port/SPI/spi.h"

/*****************************FreeRTOS INCLUDE*************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/*****************************FreeRTOS 任务配置*************************************/

// SPI数据接收任务配置
#define SPI_RECEIVER_TASK_PRIO 10             // 任务优先级
#define SPI_RECEIVER_STK_SIZE 256             // 任务堆栈大小
#define SPI_RECEIVER_TASK_NAME "SPI_Receiver" // 任务名称
TaskHandle_t xSPI_RxTask_Handler;             // 任务句柄
void vSPI_Rx_task(void *pvParameter);         // 任务函数

// 数据处理任务配置
#define ALGORITHM_TASK_PRIO 9            // 任务优先级
#define ALGORITHM_STK_SIZE 512           // 任务堆栈大小
#define ALGORITHM_TASK_NAME "Algorithm"  // 任务名称
TaskHandle_t xAlgorithmTask_Handler;     // 任务句柄
void vAlgorithm_task(void *pvParameter); // 任务函数

// 显示数据任务配置
#define SHOW_UI_TASK_PRIO 4            // 任务优先级
#define SHOW_UI_STK_SIZE 512           // 任务堆栈大小
#define SHOW_UI_TASK_NAME "ShowUI"     // 任务名称
TaskHandle_t xShowUITask_Handler;      // 任务句柄
void vShow_UI_task(void *pvParameter); // 任务函数

// 按键滤波任务配置
#define KEY_FILTER_TASK_PRIO 3           // 任务优先级
#define KEY_FILTER_STK_SIZE 128          // 任务堆栈大小
#define KEY_FILTER_TASK_NAME "KeyFilter" // 任务名称
TaskHandle_t xKeyFilterTask_Handler;     // 任务句柄
void vKeyFilter_task(void *pvParameter); // 任务函数

// START_TASK任务配置
#define START_TASK_PRIO 1            // 任务优先级
#define START_STK_SIZE 256           // 任务堆栈大小
#define START_TASK_NAME "START_TASK" // 任务名称
TaskHandle_t xStartTask_Handler;     // 任务句柄
void start_task(void *pvParameter);  // 任务函数

#if USE_FAKE_FPGA

#define FAKE_FPGA_TASK_PRIO 8           // 任务优先级
#define FAKE_FPGA_STK_SIZE 256          // 任务堆栈大小
#define FAKE_FPGA_TASK_NAME "FakeFPGA"  // 任务名称
TaskHandle_t xFakeFPGA_Task_Handler;    // 任务句柄
void vFakeFPGA_task(void *pvParameter); // 任务函数

#endif

/***********************************************************************************/
// 会用到的全局状态变量、信号量、队列等资源的句柄声明放在这里
uint16_t sKeyFlag = 0;                     // 按键滤波标志位
SemaphoreHandle_t xSem_key_Filter = NULL;  // 按键滤波信号量句柄
SemaphoreHandle_t xSem_FPGA_INT = NULL;    // FPGA中断信号量句柄
SemaphoreHandle_t xSem_SPI_Rx_Done = NULL; // SPI数据已接收信号量句柄
QueueHandle_t xQueue_Rx_Index_Buf = NULL;  // 接收数据队列句柄
QueueHandle_t xQueue_AlgoOut = NULL;       // 算法输出队列句柄
void vSemaphoreInit(void);                 // 信号量初始化函数
void vQueueInit(void);                     // 队列初始化函数
/***********************************************************************************/

/* LCD刷屏时使用的颜色 */
uint16_t lcd_discolor[11] = {WHITE, BLACK, BLUE, RED,
                             MAGENTA, GREEN, CYAN, YELLOW,
                             BROWN, BRRED, GRAY};

void RUF_X_demo(void)
{
    /* LCD显示初始化信息 */
    lcd_clear(WHITE);
#if SYS_TEST_MODE
    lcd_show_string(10, 10, 220, 32, 32, "RUF-X TEST", RED);
    lcd_show_string(10, 48, 220, 24, 24, "I'm testing!!!", BROWN);
#else
    lcd_show_string(10, 10, 220, 32, 32, "RUF-X DEMO", BLUE);
    lcd_show_string(10, 48, 220, 24, 24, "Welcome!", GREEN);
#endif

    /* 中断初始化 */
    RUF_X_exti_init();

    // 信号量初始化
    vSemaphoreInit();

    // 队列初始化
    vQueueInit();

    // SPI1初始化 其中包含DMA初始化
    vSPI1_init();

    // 创建开始任务
    xTaskCreate((TaskFunction_t)start_task,           // 任务函数
                (const char *)START_TASK_NAME,        // 任务名称
                (uint16_t)START_STK_SIZE,             // 任务堆栈大小
                (void *)NULL,                         // 任务传入参数
                (UBaseType_t)START_TASK_PRIO,         // 任务优先级
                (TaskHandle_t *)&xStartTask_Handler); // 任务句柄
    vTaskStartScheduler();
}

void start_task(void *pvParameters)
{
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

    // 创建按键滤波任务
    xTaskCreate((TaskFunction_t)vKeyFilter_task,          // 任务函数
                (const char *)KEY_FILTER_TASK_NAME,       // 任务名称
                (uint16_t)KEY_FILTER_STK_SIZE,            // 任务堆栈大小
                (void *)NULL,                             // 任务传入参数
                (UBaseType_t)KEY_FILTER_TASK_PRIO,        // 任务优先级
                (TaskHandle_t *)&xKeyFilterTask_Handler); // 任务句柄

#if USE_FAKE_FPGA
    // 创建FakeFPGA任务
    BaseType_t ret = xTaskCreate((TaskFunction_t)vFakeFPGA_task,           // 任务函数
                (const char *)FAKE_FPGA_TASK_NAME,        // 任务名称
                (uint16_t)FAKE_FPGA_STK_SIZE,             // 任务堆栈大小
                (void *)NULL,                             // 任务传入参数
                (UBaseType_t)FAKE_FPGA_TASK_PRIO,         // 任务优先级
                (TaskHandle_t *)&xFakeFPGA_Task_Handler); // 任务句柄
    printf("create FakeFPGA ret=%ld\r\n", (long)ret);
    
#endif

    vTaskDelete(xStartTask_Handler); /* 删除开始任务 */
                
//    lcd_show_string(10, 80, 220, 32, 32, "TEST_POINT_1", RED);
//    static char buf[512];
//    vTaskList(buf);
//    printf("%s\r\n", buf);
                
    taskEXIT_CRITICAL();
}

void vSemaphoreInit(void)
{
    // 创建按键滤波信号量
    xSem_key_Filter = xSemaphoreCreateBinary();
    configASSERT(xSem_key_Filter != NULL);

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
}

