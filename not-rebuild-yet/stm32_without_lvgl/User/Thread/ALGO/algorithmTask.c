/**
 * @file        algorithmtask.c
 * @brief       算法任务代码（从队列接收 FPGA 一帧数据 -> 解析 -> 计算 t1/t2/dt -> SQ统计 -> 滤波 -> 输出）
 * @note
 *  - FPGA 每次发送一帧数据包（22字节）：idxA、idxB、以及互相关 y1/y2/y3（48bit 有符号）
 *  - 本任务从 xQueue_Rx_Index_Buf 队列阻塞接收 rufx_raw_packet_t
 *  - 当前 dt 的计算单位为 ns（t1/t2/dt 都是 ns）
 */

#include "./SYSTEM/sys/sys.h"
#include "./Thread/ALGO/algorithmTask.h"
#include "./Thread/DEMO/RUF_X_demo.h"

#include "FreeRTOS.h"
#include "queue.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* ========================= 采样相关配置 ========================= */

#ifndef M_PI
#define M_PI 3.1415926535
#endif

// 一组数据的周期（ms），用于 SQ 3 秒窗口长度计算
#define GROUP_PERIOD_MS 8U
#define GROUPS_PER_SEC (1000U / GROUP_PERIOD_MS)
#define SQ_WINDOW_GROUPS (3U * GROUPS_PER_SEC) // 3 秒窗口（组数）

#define DT_S (0.04f) // 40ms 一次输出（按你当前窗口步进）

/* ========================= 阈值配置 ========================= */

// 注意：t1/t2/dt 的单位为 ns
#define T1_T2_LIMIT_NS 150000.0 // 150us = 150000ns（用于排除明显异常的 t1/t2）
#define DT_UP_LIMIT_NS 1500.0
#define DT_LOW_LIMIT_NS -25.0

/* ========================= 滑动窗滤波配置 ========================= */

#define FLOW_WINDOW_LEN 30U
#define FLOW_WINDOW_STEP 5U

/* ========================= 卡尔曼滤波 ========================= */

typedef struct
{
    float x; // 状态估计
    float p; // 估计协方差
    float q; // 过程噪声
    float r; // 测量噪声
    float k; // 卡尔曼增益
} kalman_t;

static kalman_t kf = {0.0f, 15.0f, 0.005f, 0.1f, 0.0f};

/* ========================= 数据结构定义 ========================= */

// 数据包解析后的结构（对应 FPGA 打包顺序：idxA idxB y1 y2 y3）
typedef struct
{
    uint16_t idx_a;
    uint16_t idx_b;
    int64_t conv_y1;
    int64_t conv_y2;
    int64_t conv_y3;
} rufx_packet_t;

// 算法状态
typedef struct
{
    // 0漂补偿状态（自动学习）
    float zero_offset;    // 零点偏置（单位与 v 一致，比如 m/s）
    uint16_t zero_stable; // 满足学习条件的连续次数

    // 3秒滑动窗口：记录坏数据个数（bad_flags 为环形缓冲）
    uint8_t bad_flags[SQ_WINDOW_GROUPS];
    uint16_t sq_idx;
    uint16_t sq_count;
    uint16_t sq_bad_count;

    // 流量滑动窗滤波窗口（30点，环形缓冲）
    float window_buf[FLOW_WINDOW_LEN];
    uint8_t window_idx;
    uint8_t step_cnt;
    bool window_full;

    // 配置参数（后续由配置任务更新）
    float pipe_dn; // 管径 DN（或有效内径）
    float te_ns;   // 系统固定误差（单位需与你 dt 一致，单位：ns）
    float cos_sin; // cos(theta1)*sin(theta1)

    double q_total_m3;
} algo_state_t;

static algo_state_t g_algo =
    {
        .pipe_dn = 20.0f,
        .te_ns = 12000.0f,
        .cos_sin = 0.3715f,
        .zero_offset = 0.0f,
        .zero_stable = 0,
        .q_total_m3 = 0.0};

/* ========================= 辅助函数 ========================= */

// 一维卡尔曼滤波（输入 measurement，输出平滑后的估计值）
static float run_kalman_filter(kalman_t *k, float measurement)
{
    k->p += k->q;
    k->k = k->p / (k->p + k->r);
    k->x += k->k * (measurement - k->x);
    k->p *= (1.0f - k->k);
    return k->x;
}

// 48bit 有符号数扩展为 64bit（two's complement）
static int64_t sign_extend_48(uint64_t v)
{
    // bit47 为符号位（1 表示负数）
    if (v & 0x800000000000ULL)
    {
        return (int64_t)(v | 0xFFFF000000000000ULL);
    }
    return (int64_t)v;
}

// 按 FPGA 打包格式（大端）解析 22 字节 raw 包 -> rufx_packet_t
static void rufx_unpack_packet(const rufx_raw_packet_t *raw, rufx_packet_t *out)
{
    // FPGA 打包为大端（高字节在前）
    const uint8_t *b = raw->bytes;

    // idxA / idxB
    out->idx_a = (uint16_t)((uint16_t)b[0] << 8) | (uint16_t)b[1];
    out->idx_b = (uint16_t)((uint16_t)b[2] << 8) | (uint16_t)b[3];

    // y1 / y2 / y3：48bit（6 字节，高字节到低字节）
    uint64_t y1 = ((uint64_t)b[4] << 40) | ((uint64_t)b[5] << 32) | ((uint64_t)b[6] << 24) |
                  ((uint64_t)b[7] << 16) | ((uint64_t)b[8] << 8) | (uint64_t)b[9];
    uint64_t y2 = ((uint64_t)b[10] << 40) | ((uint64_t)b[11] << 32) | ((uint64_t)b[12] << 24) |
                  ((uint64_t)b[13] << 16) | ((uint64_t)b[14] << 8) | (uint64_t)b[15];
    uint64_t y3 = ((uint64_t)b[16] << 40) | ((uint64_t)b[17] << 32) | ((uint64_t)b[18] << 24) |
                  ((uint64_t)b[19] << 16) | ((uint64_t)b[20] << 8) | (uint64_t)b[21];

    out->conv_y1 = sign_extend_48(y1);
    out->conv_y2 = sign_extend_48(y2);
    out->conv_y3 = sign_extend_48(y3);
}

/**
 * @brief  根据 idxA/idxB 与互相关 y1/y2/y3 计算 t1/t2/dt（单位：ns）
 * @note
 *  - 采样率 65MHz，采样周期 Ts = 1/65MHz ≈ 15.384615ns
 *  - t1 = idx_a * Ts
 *  - t2 = idx_b * Ts
 *  - y1/y2/y3 为互相关在 +1/0/-1 的三点值，用抛物线插值得到亚采样偏移 delta（单位：sample）
 *  - dt 按你的定义：dt = t2 - t1 + delta * Ts
 */
static bool rufx_calc_t1_t2_dt(const rufx_packet_t *pkt, double *t1, double *t2, double *dt)
{
    const double Ts_ns = 1e9 / 65e6; // 15.384615... ns per sample

    // 1) 粗时间（ns）
    *t1 = (double)pkt->idx_a * Ts_ns;
    *t2 = (double)pkt->idx_b * Ts_ns;

    // 2) 亚采样插值：delta（单位：sample）
    // y1=R(+1), y2=R(0), y3=R(-1)
    const double y1 = (double)pkt->conv_y1;
    const double y2 = (double)pkt->conv_y2;
    const double y3 = (double)pkt->conv_y3;

    const double den = (y3 - 2.0 * y2 + y1);
    if (fabs(den) < 1e-6)
    {
        return false; // 峰不明显/数值不稳定
    }

    const double delta = 0.5 * (y3 - y1) / den;

    // 可选：限制 delta 范围，防止噪声发散
    if (delta < -1.0 || delta > 1.0)
    {
        return false;
    }

    // 3) 最终 Δt（ns）：dt = t2 - t1 + delta*Ts
    *dt = (*t2 - *t1) + delta * Ts_ns;
    if (*dt > 1000.0 && *dt < 1500.0)
    {
        *dt -= 1000.0; // 你的 dt 定义里，可能需要减去 1 个采样周期（1000ns）以对齐实际物理事件
    }
    else if (*dt > 500.0 && *dt < 1000.0)
    {
        *dt -= 500.0; // 如果你定义的 dt 是基于 idxA/B 的整数采样点，那么可能需要减去 0.5 个采样周期（500ns）以对齐实际物理事件
    }

    return true;
}

// 更新 3 秒窗口坏数据统计（环形缓冲）
static void sq_window_update(algo_state_t *s, bool is_bad)
{
    if (s->sq_count < SQ_WINDOW_GROUPS)
    {
        s->bad_flags[s->sq_idx] = is_bad ? 1U : 0U;
        s->sq_bad_count += is_bad ? 1U : 0U;
        s->sq_count++;
        s->sq_idx = (uint16_t)((s->sq_idx + 1U) % SQ_WINDOW_GROUPS);
        return;
    }

    // 窗口已满：覆盖最旧数据
    s->sq_bad_count -= s->bad_flags[s->sq_idx];
    s->bad_flags[s->sq_idx] = is_bad ? 1U : 0U;
    s->sq_bad_count += is_bad ? 1U : 0U;
    s->sq_idx = (uint16_t)((s->sq_idx + 1U) % SQ_WINDOW_GROUPS);
}

// 获取 SQ（好数据百分比）
static float sq_get_percent(const algo_state_t *s)
{
    uint16_t denom = (s->sq_count == 0U) ? 1U : s->sq_count;
    float bad = (float)s->sq_bad_count;
    float total = (float)denom;
    return 100.0f * (1.0f - (bad / total));
}

// 根据 dt 计算原始流速 v（m/s）
// 公式：v = dt * L1 / (cos_sin * (t1 - te) * (t2 - te))
//
// 约定：
// - t1_ns / t2_ns / dt_ns : ns
// - te : us（会转换为 ns）
// - L1 = pipe_dn : mm
// - 输出 v : m/s
static float vel_calc_from_dt(const algo_state_t *s, double t1_ns, double t2_ns, double dt_ns)
{
    // ---- 参数准备 ----
    const double cos_sin = (double)s->cos_sin;

    // te：系统固定误差（us -> ns）
    const double te_ns = (double)s->te_ns;

    // L1：这里直接用 pipe_dn（mm）
    const double L1_mm = (double)s->pipe_dn;

    // ---- 合法性检查 ----
    const double a = t1_ns - te_ns;
    const double b = t2_ns - te_ns;

    if (cos_sin == 0.0 || a <= 0.0 || b <= 0.0)
    {
        return 0.0f;
    }

    // ---- 计算 ----
    // 单位：(ns * mm) / (ns * ns) = mm/ns
    const double v_mm_per_ns =
        (dt_ns * L1_mm) / (cos_sin * a * b);

    // mm/ns -> m/s
    const double v_mps = v_mm_per_ns * 1e6;

    return (float)v_mps;
}

static float pipe_area_m2(const algo_state_t *s)
{
    // 如果 pipe_dn 代表内径 mm
    const double D_m = (double)s->pipe_dn * 1e-3;
    const double r = 0.5 * D_m;
    return (float)(M_PI * r * r);
}

static void update_flow_outputs(algo_state_t *s, algo_out_t *out, float sq, float v_mps)
{
    // 1) 瞬时流量（m^3/s）
    const float A = pipe_area_m2(s);
    const float q_m3s = v_mps * A;

    // 2) 累计流量（m^3）
    s->q_total_m3 += (double)q_m3s * (double)DT_S;

    // 3) 打包输出
    out->sq = sq;
    out->v_mps = v_mps;
    out->q_m3s = q_m3s;
    out->q_total_m3 = s->q_total_m3;
}

/**
 * @brief  滑动窗加入新值，并在满足条件时输出 IQR 去异常均值
 * @note
 *  - 窗口长度 FLOW_WINDOW_LEN（30）
 *  - 每 FLOW_WINDOW_STEP（5）次更新输出一次
 *  - 输出时对窗口排序，按箱线图（IQR）剔除离群值，再求平均
 */
static bool flow_window_add(algo_state_t *s, float v, float *out_avg)
{
    s->window_buf[s->window_idx] = v;
    s->window_idx = (uint8_t)((s->window_idx + 1U) % FLOW_WINDOW_LEN);
    if (s->window_idx == 0U)
    {
        s->window_full = true;
    }

    s->step_cnt = (uint8_t)((s->step_cnt + 1U) % FLOW_WINDOW_STEP);
    if (s->step_cnt != 0U)
    {
        return false;
    }
    if (!s->window_full)
    {
        return false;
    }

    float temp[FLOW_WINDOW_LEN];
    memcpy(temp, s->window_buf, sizeof(temp));

    // 简单排序（窗口小，冒泡可接受）
    for (uint8_t i = 0; i < FLOW_WINDOW_LEN - 1U; i++)
    {
        for (uint8_t j = 0; j < FLOW_WINDOW_LEN - 1U - i; j++)
        {
            if (temp[j] > temp[j + 1U])
            {
                float t = temp[j];
                temp[j] = temp[j + 1U];
                temp[j + 1U] = t;
            }
        }
    }

    // Q1/Q3：取第7个和第23个（1-based）
    float q1 = temp[6];
    float q3 = temp[22];
    float iqr = q3 - q1;
    float low = q1 - 1.5f * iqr;
    float high = q3 + 1.5f * iqr;

    float sum = 0.0f;
    uint8_t cnt = 0U;
    for (uint8_t i = 0; i < FLOW_WINDOW_LEN; i++)
    {
        if (temp[i] >= low && temp[i] <= high)
        {
            sum += temp[i];
            cnt++;
        }
    }
    if (cnt == 0U)
    {
        return false;
    }

    *out_avg = sum / (float)cnt;
    return true;
}

// 自动 0漂补偿：仅在 “SQ高 + 输出接近0 + 连续稳定” 时缓慢学习 offset
static float flow_drift_comp(algo_state_t *s, float v, float sq)
{
    // ====== 参数（建议后续调参）======
    const float SQ_LEARN_MIN = 90.0f; // SQ阈值：信号质量足够好才学
    const float V_NEAR_ZERO = 0.08f;  // 近零阈值：认为可能无流（单位同 v）
    const float ALPHA = 0.005f;       // 学习速率：越小越慢越稳
    const float OFFSET_MAX = 0.5f;    // offset 安全限幅（防学歪）

    // 说明：你现在每 5 组（约40ms）才输出一次 v
    // 1秒大约 25 次输出；如果希望 6 秒后才开始学：STABLE_N ~ 150
    const uint16_t STABLE_N = 250U;

    // ====== 判断是否满足“可学习零点”的条件 ======
    if (sq >= SQ_LEARN_MIN && fabsf(v) <= V_NEAR_ZERO)
    {
        if (s->zero_stable < 0xFFFF)
            s->zero_stable++;
    }
    else
    {
        s->zero_stable = 0;
    }

    // ====== 连续稳定够久才更新 offset（避免停流瞬态/小流误判）======
    if (s->zero_stable >= STABLE_N)
    {
        // 低通学习：offset 缓慢追踪 v（零流时 v≈offset）
        s->zero_offset = (1.0f - ALPHA) * s->zero_offset + ALPHA * v;

        // 安全限幅
        if (s->zero_offset > OFFSET_MAX)
            s->zero_offset = OFFSET_MAX;
        if (s->zero_offset < -OFFSET_MAX)
            s->zero_offset = -OFFSET_MAX;
    }

    // 输出补偿后的结果
    return v - s->zero_offset;
}

// 最终限幅/死区处理
static float flow_limit(float v)
{
    if (fabsf(v) < 0.05f)
    {
        return 0.0f;
    }
    if (fabsf(v) > 5.0f)
    {
        return 0.0f;
    }
    return v;
}

// 处理一组 t1/t2/dt（SQ统计 + dt->flow + 滤波链）
static void handle_group(algo_state_t *s, algo_out_t *out, double t1_ns, double t2_ns, double dt_ns)
{
    // 判坏：t1/t2 超限 或 dt 超限
    bool is_bad = (t1_ns > T1_T2_LIMIT_NS) ||
                  (t2_ns > T1_T2_LIMIT_NS) ||
                  (dt_ns > DT_UP_LIMIT_NS) ||
                  (dt_ns < DT_LOW_LIMIT_NS);

    sq_window_update(s, is_bad);

    float sq = sq_get_percent(s);

    float flow_v_raw = vel_calc_from_dt(s, t1_ns, t2_ns, dt_ns); // dt -> v

    // 30点 IQR 均值（每5点输出一次）
    float flow_v_avg = 0.0f;
    if (!flow_window_add(s, flow_v_raw, &flow_v_avg))
    {
        return;
    }

    // 卡尔曼平滑
    float flow_v_kalman = run_kalman_filter(&kf, flow_v_avg);

    // 0漂补偿（TODO）
    float flow_v_comp = flow_drift_comp(s, flow_v_kalman, sq);

    // 最终限幅/死区
    float flow_v_final = flow_limit(flow_v_comp);

    update_flow_outputs(s, out, sq, flow_v_final);
}

/* ========================= FreeRTOS 任务入口 ========================= */

void vAlgorithm_task(void *pvParameter)
{
    (void)pvParameter;

    rufx_raw_packet_t raw = {0};
    algo_out_t out = {0.0f, 0.0f, 0.0f, 0.0};

    while (1)
    {
        // 阻塞等待 SPI 任务通过队列送来的原始数据包
        if (xQueueReceive(xQueue_Rx_Index_Buf, &raw, portMAX_DELAY) == pdTRUE)
        {
            // 1) 拆包
            rufx_packet_t pkt;
            rufx_unpack_packet(&raw, &pkt);

            // 2) 计算 t1/t2/dt（ns）
            double t1 = 0.0;
            double t2 = 0.0;
            double dt = 0.0;

            if (!rufx_calc_t1_t2_dt(&pkt, &t1, &t2, &dt))
            {
                continue;
            }

            // 3) 处理一组数据
            handle_group(&g_algo, &out, t1, t2, dt);
            (void)xQueueOverwrite(xQueue_AlgoOut, &out);
        }
    }
}
