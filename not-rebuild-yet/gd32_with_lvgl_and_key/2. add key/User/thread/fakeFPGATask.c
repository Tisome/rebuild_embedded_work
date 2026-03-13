/**
 * @brief
 *
 * @param pvParameter
 */

#include "app_config.h"

#include "fakeFPGATask.h"
#include "algorithmTask.h"
#include "RUF_X_demo.h"

#include "freertos_resources.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#if RCT6
#include "led.h"
#include <stdio.h>
#include "dma.h"
#endif

void pack48(uint8_t *buf, int64_t value)
{
    buf[0] = (value >> 40) & 0xFF;
    buf[1] = (value >> 32) & 0xFF;
    buf[2] = (value >> 24) & 0xFF;
    buf[3] = (value >> 16) & 0xFF;
    buf[4] = (value >> 8) & 0xFF;
    buf[5] = value & 0xFF;
}

// void vFakeFPGA_task(void *pvParameter)
//{
//     (void)pvParameter;

//    rufx_raw_packet_t pkt;
//    memset(&pkt, 0, sizeof(pkt));

//    uint16_t idx = 1000;

//    while (1)
//    {
//        // 模拟 idx 变化
//        pkt.bytes[0] = (3000) >> 8;
//        pkt.bytes[1] = (3000) & 0xFF;

//        pkt.bytes[2] = (3000 + 1 + idx % 2) >> 8;
//        pkt.bytes[3] = (3000 + 1 + idx % 2) & 0xFF;

//        // 模拟互相关 y1 y2 y3
//        int64_t y1 = 900 + 3 * idx % 17;
//        int64_t y2 = 1200 + 2 * idx % 11; // 峰值
//        int64_t y3 = 950 + 5 * idx % 37;

//        // 打包48bit
//        pack48(&pkt.bytes[4], y1);
//        pack48(&pkt.bytes[10], y2);
//        pack48(&pkt.bytes[16], y3);

//        BaseType_t ok = xQueueSend(xQueue_Rx_Index_Buf, &pkt, 0);
// #if RCT6
//        printf("send=%ld\r\n", (long)ok);
// #endif
//        vTaskDelay(pdMS_TO_TICKS(8)); // 模拟8ms一包
//        idx++;
//    }
//}

// void vFakeFPGA_task(void *pvParameter)
// {
//     (void)pvParameter;

//     rufx_raw_packet_t pkt;
//     memset(&pkt, 0, sizeof(pkt));

//     uint16_t idx = 1000;

//     // 写入本次发送的buffer地址和长度
//     dma_memory_address_config(DMA0, DMA_CH2, (uint32_t)&pkt);
//     dma_transfer_number_config(DMA0, DMA_CH2, (uint32_t)22);
//     dma_memory_increase_enable(DMA0, DMA_CH2);

//     while (1)
//     {
//         // 模拟 idx 变化
//         pkt.bytes[0] = (3000) >> 8;
//         pkt.bytes[1] = (3000) & 0xFF;

//         pkt.bytes[2] = (3000 + 1 + idx % 2) >> 8;
//         pkt.bytes[3] = (3000 + 1 + idx % 2) & 0xFF;

//         // 模拟互相关 y1 y2 y3
//         int64_t y1 = 900 + 3 * idx % 17;
//         int64_t y2 = 1200 + 2 * idx % 11; // 峰值
//         int64_t y3 = 950 + 5 * idx % 37;

//         // 打包48bit
//         pack48(&pkt.bytes[4], y1);
//         pack48(&pkt.bytes[10], y2);
//         pack48(&pkt.bytes[16], y3);

//         LED_TOGGLE();
//         idx++;
//         printf("fake data\r\n");

//         xSemaphoreGive(xSem_FPGA_INT);
//         vTaskDelay(pdMS_TO_TICKS(80)); // 模拟80ms一包,实际上是8ms一包
//     }
// }

#include <math.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FAKE_PIPE_DN 20.0f
#define FAKE_TE_NS 12000.0f
#define FAKE_COS_SIN 0.3715f

typedef struct
{
    float v_min;    // m/s
    float v_max;    // m/s
    float period_s; // s
} fake_sig_cfg_t;

static volatile fake_sig_cfg_t g_fake_cfg = {
    .v_min = 1.0f,
    .v_max = 4.0f,
    .period_s = 10.0f,
};

static float fake_sig_sine(float t, const fake_sig_cfg_t *cfg)
{
    float mid = 0.5f * (cfg->v_min + cfg->v_max);
    float amp = 0.5f * (cfg->v_max - cfg->v_min);
    float w = 2.0f * 3.1415926f / (cfg->period_s > 0.001f ? cfg->period_s : 0.001f);
    return mid + amp * sinf(w * t);
}



/* ====== 48-bit pack helpers ====== */
static uint64_t pack_s48(int64_t v)
{
    return (uint64_t)v & 0xFFFFFFFFFFFFULL; // two's complement truncated
}

static void put_be48(uint8_t *p, int64_t s48)
{
    uint64_t u = pack_s48(s48);
    p[0] = (u >> 40) & 0xFF;
    p[1] = (u >> 32) & 0xFF;
    p[2] = (u >> 24) & 0xFF;
    p[3] = (u >> 16) & 0xFF;
    p[4] = (u >> 8) & 0xFF;
    p[5] = (u >> 0) & 0xFF;
}

/* ====== build y1/y2/y3 so that parabola interpolation returns "delta" ======
   Your algo uses:
     delta = 0.5 * (y3 - y1) / (y3 - 2*y2 + y1)
   with y1=R(+1), y2=R(0), y3=R(-1).
   Let R(x)=A - K*(x - delta)^2, then this formula returns exactly delta.
*/
static void make_parabola_3pts(double delta, int64_t *y1, int64_t *y2, int64_t *y3)
{
    const double A = 1e9;                                 // peak magnitude
    const double K = 1e8;                                 // curvature
    double r0 = A - K * (0.0 - delta) * (0.0 - delta);    // R(0)
    double rp1 = A - K * (1.0 - delta) * (1.0 - delta);   // R(+1) => y1
    double rm1 = A - K * (-1.0 - delta) * (-1.0 - delta); // R(-1) => y3

    *y1 = (int64_t)llround(rp1);
    *y2 = (int64_t)llround(r0);
    *y3 = (int64_t)llround(rm1);
}

/* ====== invert v -> dt -> idxB/delta, then pack raw[22] ======
   Assumptions:
   - LVGL etc irrelevant, this is for algorithmTask input.
   - idx_a chosen externally and increases over time.
   - Uses g_algo params: pipe_dn(mm), te_ns(ns), cos_sin.
*/
static void fake_make_packet_from_v(rufx_raw_packet_t *raw,
                                    uint16_t idx_a,
                                    float v_mps,
                                    float pipe_dn_mm,
                                    float te_ns,
                                    float cos_sin)
{
    const double Ts_ns = 1e9 / 65e6; // 15.384615 ns/sample

    // t1 from idx_a
    double t1_ns = (double)idx_a * Ts_ns;

    // Make sure (t1 - te) positive
    double a = t1_ns - (double)te_ns;
    if (a < 1000.0)
        a = 1000.0; // guard

    // We don't know t2 yet; dt is tiny vs t1, so approximate b≈a is fine.
    double b = a;

    // v(m/s) -> v(mm/ns)
    double v_mm_per_ns = (double)v_mps * 1e-6;

    // Invert your vel formula:
    // v_mm/ns = dt_ns * L1_mm / (cos_sin * a * b)
    // => dt_ns = v_mm/ns * (cos_sin * a * b) / L1_mm
    double dt_ns = v_mm_per_ns * ((double)cos_sin * a * b) / (double)pipe_dn_mm;

    // Convert dt to samples
    double d = dt_ns / Ts_ns;

    // Choose integer diff + fractional delta in [-0.5, 0.5] (more stable)
    int di = (int)floor(d + 0.5);  // nearest integer
    double delta = d - (double)di; // now in [-0.5, 0.5]

    // Clamp delta to algo's acceptance (-1..1), keep it safe
    if (delta < -0.95)
        delta = -0.95;
    if (delta > 0.95)
        delta = 0.95;

    uint16_t idx_b = (uint16_t)(idx_a + di);

    // Build y1/y2/y3 that yields this delta
    int64_t y1, y2, y3;
    make_parabola_3pts(delta, &y1, &y2, &y3);

    // Pack big-endian as your unpack expects
    uint8_t *bptr = raw->bytes;
    bptr[0] = (uint8_t)(idx_a >> 8);
    bptr[1] = (uint8_t)(idx_a);
    bptr[2] = (uint8_t)(idx_b >> 8);
    bptr[3] = (uint8_t)(idx_b);

    put_be48(&bptr[4], y1);
    put_be48(&bptr[10], y2);
    put_be48(&bptr[16], y3);
}

void vFakeFPGA_task(void *pvParameter)
{
    (void)pvParameter;

    TickType_t t0 = xTaskGetTickCount();
    const uint16_t idx_a = 3000;   // 固定，不再跨帧累加

    for (;;)
    {
        TickType_t now = xTaskGetTickCount();
        float t = (float)(now - t0) * portTICK_PERIOD_MS / 1000.0f;

        fake_sig_cfg_t cfg = g_fake_cfg;
        float v = fake_sig_sine(t, &cfg);

        rufx_raw_packet_t raw;
        memset(&raw, 0, sizeof(raw));
        fake_make_packet_from_v(&raw,
                                idx_a,
                                v,
                                FAKE_PIPE_DN,
                                FAKE_TE_NS,
                                FAKE_COS_SIN);

        (void)xQueueSend(xQueue_Rx_Index_Buf, &raw, 0);

        vTaskDelay(pdMS_TO_TICKS(8));
    }
}
