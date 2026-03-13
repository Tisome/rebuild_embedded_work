
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./Thread/DEMO/RUF_X_demo.h"
#include "./Thread/SHOW_UI/show_UI.h"
#include "./Thread/ALGO/algorithmTask.h"
#include "./CONFIG/app_config.h"

#include "FreeRTOS.h"
#include "task.h"

void vShow_UI_task(void *pvParameter)
{
    (void)pvParameter;
    algo_out_t out = {0.0f, 0.0f, 0.0f, 0.0f};
    static uint32_t cnt = 0;
    while (1)
    {

        // 阻塞等最新输出
        if (xQueueReceive(xQueue_AlgoOut, &out, portMAX_DELAY) == pdTRUE)
        {
#if USE_UART_LOG
            if (++cnt % 10 == 0) // 每10次输出打印一次
            {
                printf("SQ=%.1f%%, v=%.4f m/s, Q=%.6f m3/s, Total=%.4f m3\r\n",
                       out.sq, out.v_mps, out.q_m3s, out.q_total_m3);
            }
#else
            // TODO: 未来 LVGL / LCD 显示更新放这里
#endif
        }
        else
        {
            // 超时未收到新数据，目前不用做事
        }
    }
}


