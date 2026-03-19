#include "app_config.h"
#include "data.h"
#include "freertos_resources.h"
#include "task_manager.h"

#include "FreeRTOS.h"
#include "bsp_key.h"
#include "queue.h"
#include "sys.h"

#include "elog.h"
#define LOG_TAG "key_task"
#define LOG_LVL ELOG_LVL_VERBOSE

static TaskHandle_t task_key_handle = NULL;

TaskHandle_t get_bsp_key_task_handle(void)
{
    return task_key_handle;
}

void do_create_bsp_key_task(void)
{
    BaseType_t result = pdPASS;
    result = xTaskCreate(task_key,
                         "key_task",
                         256,
                         NULL,
                         2,
                         &task_key_handle);
    if (result != pdPASS)
    {
        log_e("Failed to create key task");
    }
}