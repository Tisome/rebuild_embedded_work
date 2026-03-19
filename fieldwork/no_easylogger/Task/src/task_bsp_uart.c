#include "bsp_usart.h"
#include "task_manager.h"

static TaskHandle_t task_uart_handle = NULL;

void do_create_bsp_uart_task(void)
{
    BaseType_t result = pdPASS;
    result = xTaskCreate(task_uart_driver,
                         "uart_task",
                         512,
                         NULL,
                         2,
                         &task_uart_handle);
    if (result != pdPASS)
    {
        log_e("Failed to create UART task");
    }
}
TaskHandle_t get_bsp_uart_task_handle(void)
{
    return task_uart_handle;
}