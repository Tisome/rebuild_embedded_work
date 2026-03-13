#include "data.h"
#include "elog.h"

kalman_t kf = {0.0f, 15.0f, 0.005f, 0.1f, 0.0f};

algo_state_t g_algo =
    {
        .pipe_dn = 20.0f,
        .te_ns = 12000.0f,
        .cos_sin = 0.3715f,
        .zero_offset = 0.0f,
        .zero_stable = 0,
        .q_total_m3 = 0.0};

e2prom_status_t saveParameters()