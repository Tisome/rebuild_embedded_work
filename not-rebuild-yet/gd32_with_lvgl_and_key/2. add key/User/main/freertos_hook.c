#include "gd32f30x.h"
#include "FreeRTOS.h"
#include "task.h"

/* 让调试器能看到是谁炸了栈 */
volatile const char *g_freertos_fault_task_name = 0;
volatile UBaseType_t g_freertos_fault_task_prio = 0;
volatile uint32_t g_freertos_fault_reason = 0; /* 1=stack overflow, 2=malloc failed */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* 记录信息（不要printf） */
    g_freertos_fault_reason = 1;
    g_freertos_fault_task_name = pcTaskName;
    g_freertos_fault_task_prio = uxTaskPriorityGet(xTask);

    /* 关中断，防止继续乱跑 */
    taskDISABLE_INTERRUPTS();

    /* 可选：点亮错误灯（你有 LED 的话） */
    /* BSP_ErrorLedOn(); */

    /* 开发期建议：死循环等待调试器 */
    for (;;)
    {
        __NOP();
    }
}

void vApplicationMallocFailedHook(void)
{
    g_freertos_fault_reason = 2;

    taskDISABLE_INTERRUPTS();

    /* 可选：错误指示 */
    /* BSP_ErrorLedOn(); */

    for (;;)
    {
        __NOP();
    }
}


