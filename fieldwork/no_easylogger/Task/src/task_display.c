#include "app_config.h"
#include "data.h"
#include "freertos_resources.h"
#include "task_manager.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "sys.h"

#include "elog.h"
#define LOG_TAG "display_task"
#define LOG_LVL ELOG_LVL_VERBOSE

static TaskHandle_t task_display_handle = NULL;

void task_display(void *pvParameter)
{
    (void)pvParameter;

    while (1)
    {
        if (xQueueReceive(xQueue_AlgoOut, &g_algo_out, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            log_v("receive algo output");
        }
    }
}