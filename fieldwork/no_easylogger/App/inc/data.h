/* ========================= 采样相关配置 ========================= */

#ifndef M_PI
#define M_PI 3.1415926535
#endif

// 一组数据的周期（ms），用于 SQ 3 秒窗口长度计算
#define GROUP_PERIOD_MS  8U
#define GROUPS_PER_SEC   (1000U / GROUP_PERIOD_MS)
#define SQ_WINDOW_GROUPS (3U * GROUPS_PER_SEC) // 3 秒窗口（组数）

#define DT_S             (0.04f) // 40ms 一次输出（按你当前窗口步进）

/* ========================= 阈值配置 ========================= */

// 注意：t1/t2/dt 的单位为 ns
#define T1_T2_LIMIT_NS  150000.0 // 150us = 150000ns（用于排除明显异常的 t1/t2）
#define DT_UP_LIMIT_NS  1500.0
#define DT_LOW_LIMIT_NS -25.0

/* ========================= 滑动窗滤波配置 ========================= */

#define FLOW_WINDOW_LEN  30U
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

extern kalman_t kf;

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

typedef enum {
    PIPE_PVC   = 0, // PVC管道
    PIPE_METAL = 1, // 钢管管道
    PIPE_ALLOY = 2  // 铜管管道
} PipeType;

typedef struct
{
    // 8字节对齐
    double outer_diameter;         // 外径
    double inner_diameter;         // 内径
    double cos_value;              // cos值
    double sin_value;              // sin值
    double lower_range;            // 计算截断下限流量
    double upper_range;            // 计算截断上限流量
    double alarm_lower_range;      // 警报下限流量
    double alarm_upper_range;      // 警报上限流量
    double modbus_addr;            // 暂定modbus用
    double k_factor;               // 暂定modbus用
    double zero_offset_speed;      // 0漂补偿计算速度
    double zero_start_learn_speed; // 0漂补偿开始学习速度

    // 4字节对齐
    uint32_t is_saved;            // 当前参数是否保存
    uint32_t flow_speed_unit;     // 流速单位
    uint32_t flow_rate_unit;      // 流量单位
    uint32_t output_mode;         // 输出模式
    uint32_t display_sensitivity; // 显示/上传刷新频率
    uint32_t zero_offset_stable;  // 0漂补偿满足学习的稳定次数

    // 自设定类型
    PipeType pipe_type; // 管道类型

} Pipe_Parameter_t;

typedef struct
{
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

} algo_state_t;

extern algo_state_t g_algo;

typedef struct
{
    double flow_speed;
    double flow_rate_instant;
    double flow_rate_total;
    double sq_value;
} algo_res_data;