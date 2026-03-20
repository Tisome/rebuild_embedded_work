#include "task_manager.h"
static TaskHandle_t task_modbus_execute_handle = NULL;

void do_create_modbus_execute_task(void)
{
    BaseType_t result = pdPASS;
    result = xTaskCreate(task_modbus_execute,
                         "modbus_parse_task",
                         512,
                         NULL,
                         2,
                         &task_modbus_execute_handle);
    if (result != pdPASS)
    {
        log_e("Failed to create Modbus parse task");
    }
}
TaskHandle_t get_modbus_execute_task_handle(void)
{
    return task_modbus_execute_handle;
}