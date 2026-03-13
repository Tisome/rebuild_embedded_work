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
    float sq;         // 信号质量 (%)
    float v_mps;      // 流速 (m/s)
    float q_m3s;      // 体积流量 (m^3/s)
    float q_total_m3; // 累计体积流量 (m^3)
} algo_out_t;

typedef enum
{
    METAL = 0,
    PLASTIC,
    ALUMINUM
} Material;

/* ================= 算法状态结构 ================= */

#define SQ_WINDOW_GROUPS 375
#define FLOW_WINDOW_LEN 30

typedef struct
{
    float zero_offset;
    uint16_t zero_stable;

    uint8_t bad_flags[SQ_WINDOW_GROUPS];
    uint16_t sq_idx;
    uint16_t sq_count;
    uint16_t sq_bad_count;

    float window_buf[FLOW_WINDOW_LEN];
    uint8_t window_idx;
    uint8_t step_cnt;
    bool window_full;

    Material material;
    float pipe_dn;
    float te_ns;
    float cos_sin;

    double q_total_m3;

} algo_state_t;

/* ================= 全局变量 ================= */

extern algo_state_t g_algo;

/* ================= API ================= */

void vAlgorithm_task(void *pvParameter);

void diameter_set_dn(float dn);
void material_set(Material m);

void clear_data(void);
void zero_drift_learn_start(void);

#endif