#include "algorithm_flow.h"
#include "algorithm_packet.h"
#include "algorithm_process.h"

#include "app_config.h"
#include "data.h"
#include "freertos_resources.h"
#include "task_manager.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "sys.h"

#include <stdbool.h>

#include "elog.h"
#define LOG_TAG "algo_task"
#define LOG_LVL ELOG_LVL_VERBOSE

static TaskHandle_t task_algorithm_handle = NULL;

void task_algorithm(void *pvParameter)
{
    (void)pvParameter;

    rufx_raw_packet_t raw = {0};
    uint8_t cur_seq = 0;
    uint8_t last_seq = 0;

    parameter_init();

    while (1)
    {
        if (xQueueReceive(xQueue_Rx_Index_Buf, &raw, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            log_v("receive rufx raw packet");
            g_alarm = ALARM_OK;

            rufx_packet_t pkt;
            rufx_unpack_packet(&raw, &pkt, &cur_seq);

            if (cur_seq == last_seq)
            {
                log_e("repeat packet!");
                g_alarm = ALARM_REPEAT_PACKET;
            }
            last_seq = cur_seq;

            double t1 = 0.0, t2 = 0.0, dt = 0.0;
            if (!rufx_calc_t1_t2_dt(&pkt, &t1, &t2, &dt))
            {
                continue;
            }

            bool has_new_output = algorithm_process_group(&g_parameters,
                                                          &g_algo_state,
                                                          &g_algo_out,
                                                          t1,
                                                          t2,
                                                          dt);
            if (has_new_output)
            {
                log_i("send new algo_out data");
                (void)xQueueOverwrite(xQueue_AlgoOut, &g_algo_out);
            }
        }
        else
        {
            log_e("rufx raw packet out of time");
            g_alarm = ALARM_OUT_OF_TIME;
        }
    }
}

TaskHandle_t get_algorithm_task_handle(void)
{
    return task_algorithm_handle;
}

void do_create_algorithm_task(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(task_algorithm,
                          "task_algorithm",
                          512,
                          NULL,
                          9,
                          &task_algorithm_handle);
    (void)xReturn;
}