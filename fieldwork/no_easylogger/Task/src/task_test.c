#include "data.h"
#include "fake_data.h"
#include "freertos_resources.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "elog.h"
#define LOG_TAG "test"
#define LOG_LVL ELOG_LVL_VERBOSE

static TaskHandle_t task_test_handle = NULL;

void task_test(void *pvParameter)
{
    (void)pvParameter;

    TickType_t t0 = xTaskGetTickCount();
    uint8_t seq = 0;

    log_i("test_task start");
    rufx_raw_packet_t raw = {0};
    while (1)
    {
        TickType_t now = xTaskGetTickCount();
        float t_s = (float)(now - t0) * portTICK_PERIOD_MS / 1000.0f;

        fake_data_make_packet(&raw, t_s, &g_parameters);
        raw.seq = seq++;

        log_v("send fake data");
        (void)xQueueOverwrite(xQueue_Rx_Index_Buf, &raw);

        vTaskDelay(pdMS_TO_TICKS(8));
    }
}

TaskHandle_t get_test_task_handle(void)
{
    return task_test_handle;
}

void do_create_test_task(void)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate(task_test,
                          "task_test",
                          256,
                          NULL,
                          9,
                          &task_test_handle);

    (void)xReturn;
}
