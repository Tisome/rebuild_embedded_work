#include "data.h"
#include "elog.h"

Pipe_Parameters_t g_parameters;

kalman_t kf = {0.0, 15.0, 0.005, 0.1, 0.0};

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
};

Pipe_algo_out_data_t g_algo_out = {0.0, 0.0, 0.0, 0.0};

algo_state_t g_algo =
    {
        .pipe_dn = 20.0f,
        .te_ns = 12000.0f,
        .cos_sin = 0.3715f,
        .zero_offset = 0.0f,
        .zero_stable = 0,
        .q_total_m3 = 0.0};

ALARM_TYPE g_alarm = ALARM_OK;

// 初始化参数变量
void parameter_init(void)
{
    Pipe_Parameters_t default_pipe_parameters =
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
            .rate_unit_type = RATE_UNIT_M3_P_H,
        }
#if USE_EEPROM
    LoadParameter(&g_parameters);
    if (g_parameters.is_saved != 1)
    {
        SaveParameter(&default_pipe_parameters);
    }
    LoadParameter(&g_parameters);
#else
    memcpy(&g_parameters, &default_pipe_parameters, sizeof(Pipe_Parameters_t));
#endif
}

e2prom_status_t SaveParameters(Pipe_Parameters_t *para)
{
}

e2prom_status_t LoadParameters(Pipe_Parameters_t *para)
{
}