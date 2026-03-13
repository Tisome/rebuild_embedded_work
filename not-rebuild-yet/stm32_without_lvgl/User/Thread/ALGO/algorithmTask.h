#ifndef __ALGORITHM_TASK_H
#define __ALGORITHM_TASK_H

#include <stdbool.h>
#include <stdint.h>

#define RUF_X_PACKET_SIZE_BYTES 22

typedef struct
{
    uint8_t bytes[RUF_X_PACKET_SIZE_BYTES];
} rufx_raw_packet_t;

typedef struct
{
    float sq;          // 信号质量 (%)
    float v_mps;       // 流速 (m/s)
    float q_m3s;       // 体积流量 (m^3/s)
    double q_total_m3; // 累计体积流量 (m^3)
} algo_out_t;

void vAlgorithm_task(void *pvParameter);

#endif
