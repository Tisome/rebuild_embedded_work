/**
 * @brief
 *
 * @param pvParameter
 */

#include "./Thread/FakeFPGA/fakeFPGATask.h"
#include "./Thread/ALGO/algorithmTask.h"
#include "./Thread/DEMO/RUF_X_demo.h"
#include "./SYSTEM/usart/usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <string.h>

void pack48(uint8_t *buf, int64_t value)
{
    buf[0] = (value >> 40) & 0xFF;
    buf[1] = (value >> 32) & 0xFF;
    buf[2] = (value >> 24) & 0xFF;
    buf[3] = (value >> 16) & 0xFF;
    buf[4] = (value >> 8) & 0xFF;
    buf[5] = value & 0xFF;
}

void vFakeFPGA_task(void *pvParameter)
{
    (void)pvParameter;

    rufx_raw_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    uint16_t idx = 1000;

    while (1)
    {
        // 模拟 idx 变化
        pkt.bytes[0] = (3000) >> 8;
        pkt.bytes[1] = (3000) & 0xFF;

        pkt.bytes[2] = (3000 + 1 + idx % 2) >> 8;
        pkt.bytes[3] = (3000 + 1 + idx % 2) & 0xFF;

        // 模拟互相关 y1 y2 y3
        int64_t y1 = 900 + 3* idx % 17;
        int64_t y2 = 1200 +  2 * idx % 11; // 峰值
        int64_t y3 = 950 + 5 * idx % 37;

        // 打包48bit
        pack48(&pkt.bytes[4], y1);
        pack48(&pkt.bytes[10], y2);
        pack48(&pkt.bytes[16], y3);

        BaseType_t ok = xQueueSend(xQueue_Rx_Index_Buf, &pkt, 0);
        printf("send=%ld\r\n", (long)ok);
        vTaskDelay(pdMS_TO_TICKS(8)); // 模拟8ms一包
        idx++;
    }
}
