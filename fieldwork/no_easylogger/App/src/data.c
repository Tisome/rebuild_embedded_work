#include "data.h"

#include <stdbool.h>

/* ========================= 全局数据定义 ========================= */

Pipe_Parameters_t g_parameters = {0};

kalman_t kf =
    {
        .x = 0.0,
        .p = 15.0,
        .q = 0.005,
        .r = 0.1,
        .k = 0.0};

Pipe_algo_state_t g_algo_state =
    {
        .zero_stable = 0,

        .bad_flags = {0},
        .sq_idx = 0,
        .sq_count = 0,
        .sq_bad_count = 0,

        .window_buf = {0},
        .window_idx = 0,
        .step_cnt = 0,
        .window_full = false,

        .q_total_m3 = 0.0};

Pipe_algo_out_data_t g_algo_out =
    {
        .flow_speed = 0.0,
        .flow_rate_instant = 0.0,
        .flow_rate_total = 0.0,
        .sq_value = 0.0};

ALARM_TYPE g_alarm = ALARM_OK;

/* ========================= 参数初始化 ========================= */

void parameter_init(void)
{
    const Pipe_Parameters_t default_pipe_parameters =
        {
            .inner_diameter = 20.0,
            .wall_thick = 1.0,
            .cos_value = 0.913545,
            .sin_value = 0.406738,

            .lower_speed_range = 0.05,
            .upper_speed_range = 20.0,

            .alarm_lower_rate_range = 0.3,
            .alarm_upper_rate_range = 4.8,

#if USE_MODBUS
            .modbus_addr = 1,
            .k_factor = 5,
#endif

            .zero_offset_speed = 0.0,
            .zero_learn_flow_speed = 0.08,
            .zero_learn_alpha = 0.005,
            .zero_learn_offset_max = 0.2,
            .zero_learn_sq_min = 95.0,

            .te_ns = 12000.0,

            .is_saved = 0,
            .output_mode = 0,
            .display_sensitivity = 5,
            .zero_stable_threshold = 150,

            .pipe_type = PIPE_PVC,
            .speed_unit_type = SPEED_UNIT_M_P_S,
            .rate_unit_type = RATE_UNIT_M3_P_H};

#if USE_EEPROM
    if (LoadParameters(&g_parameters) != E2PROM_OK)
    {
        g_parameters = default_pipe_parameters;
        return;
    }

    if (g_parameters.is_saved != 1U)
    {
        (void)SaveParameters((Pipe_Parameters_t *)&default_pipe_parameters);
        (void)LoadParameters(&g_parameters);
    }
#else
    g_parameters = default_pipe_parameters;
#endif
    log_v("parameter loaded");
}

/* ========================= EEPROM 参数接口 ========================= */

e2prom_status_t SaveParameters(Pipe_Parameters_t *para)
{
    if (para == NULL)
    {
        return E2PROM_ERROR;
    }

    /* TODO:
       1. 将 *para 按字节写入 EEPROM
       2. 建议附带版本号 / CRC / is_saved 标志
       3. 写成功后返回 E2PROM_OK
    */

    return E2PROM_OK;
}

e2prom_status_t LoadParameters(Pipe_Parameters_t *para)
{
    if (para == NULL)
    {
        return E2PROM_ERROR;
    }

    /* TODO:
       1. 从 EEPROM 读出参数到 *para
       2. 校验数据是否合法（可加 CRC / magic）
       3. 读成功返回 E2PROM_OK
    */

    return E2PROM_OK;
}
