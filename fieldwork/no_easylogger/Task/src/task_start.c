#include "task_manager.h"

static TaskHandle_t task_start_handle = NULL; /* 创建任务句柄 */
static void task_start(void *params)
{
    taskENTER_CRITICAL();
    do_create_bsp_e2prom_task();
    do_create_bsp_uart_task();
    do_create_key_task();
    do_create_display_task();
    do_create_spi_rx_task();
    do_create_algorithm_task();
    do_create_watchdog_task();

    taskEXIT_CRITICAL(); // 提前退出临界区
    vTaskDelete(task_start_handle);
    // 启动系统调度
    vTaskStartScheduler();
}

void do_create_start_task(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(task_start, "task_start", 128, NULL, 1, &task_start_handle);
}