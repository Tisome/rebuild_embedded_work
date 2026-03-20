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

    Pipe_algo_out_data_t data;

    while (1)
    {
        if (xQueueReceive(xQueue_AlgoOut, &data, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            log_v("receive algo output");
            log_i("----- Pipe Algo Output -----");
            log_i("flow_speed        : %.3f %s",
                  data.flow_speed,
                  speed_unit_to_str(data.flow_speed_unit));
            log_i("flow_rate_instant : %.3f %s",
                  data.flow_rate_instant,
                  rate_unit_to_str(data.flow_rate_unit));
            log_i("flow_rate_total   : %.3f %s",
                  data.flow_rate_total,
                  volume_unit_to_str(data.flow_total_unit));
            log_i("sq_value          : %.3f %%", data.sq_value);
        }
        else
        {
            log_e("task display receive data error");
        }
    }
}

void do_create_display_task(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */
    /* 创建AppTaskCreate任务 */
    xReturn = xTaskCreate(task_display,
                          "task_display",
                          128,
                          NULL,
                          9,
                          &task_display_handle);
    if (xReturn != pdPASS)
    {
        log_e("Failed to create task display");
    }
}

TaskHandle_t get_display_task_handle(void)
{
    return task_display_handle;
}
