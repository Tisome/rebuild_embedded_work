#include "data.h"
#include "elog.h"
Parameters_t g_parameters;

MaterialConfig g_materialConfigs[9][3] = {
    // 第0行：DN0~13
    {
        {11, 1.1296, 0x0D80, 7, 4, 13.0}, // plastic
        {11, 1.4694, 0x0D80, 0, 4, 13.0}, // metal
        {11, 1.7326, 0x0D80, 7, 4, 13.0}  // alloy
    },

    // 第1行：DN13~18
    {
        {11, 1.1296, 0x1300, 7, 10, 18.0},  // plastic
        {11, 1.4694, 0x1300, 15, 10, 18.0}, // metal
        {11, 1.7326, 0x1300, 7, 10, 18.0}   // alloy
    },

    // 第2行：DN18~23
    {
        {7, 1.1296, 0x1300, 7, 10, 23.0},  // plastic
        {4, 1.4694, 0x1400, 15, 10, 23.0}, // metal
        {3, 1.7326, 0x1500, 15, 10, 23.0}  // alloy
    },

    // 第3行：DN23~28
    {
        {6, 1.1296, 0x1980, 7, 10, 28.0},  // plastic
        {6, 1.4694, 0x1980, 15, 10, 28.0}, // metal
        {6, 1.7326, 0x1980, 15, 10, 28.0}  // alloy
    },

    // 第4行：DN28~33
    {
        {9, 1.1296, 0x1B00, 7, 10, 33.0},   // plastic
        {9, 1.4694, 0x1B00, 15, 10, 33.0},  // metal
        {11.5, 1.7326, 0x1B00, 7, 10, 33.0} // alloy
    },

    // 第5行：DN33~38
    {
        {7, 1.1296, 0x1F00, 7, 10, 38.0},    // plastic
        {4.5, 1.4694, 0x1F00, 15, 10, 38.0}, // metal
        {4, 1.7326, 0x1F00, 7, 10, 38.0}     // alloy
    },

    // 第6行：DN38~43
    {
        {7, 1.1296, 0x2280, 7, 10, 43.0},  // plastic
        {7, 1.4694, 0x2280, 15, 10, 43.0}, // metal
        {6, 1.7326, 0x2280, 7, 10, 43.0}   // alloy
    },

    // 第7行：DN43~48
    {
        {9, 1.1296, 0x2600, 7, 10, 48.0},  // plastic
        {9, 1.4694, 0x2600, 15, 10, 48.0}, // metal
        {7.5, 1.7326, 0x2600, 7, 10, 48.0} // alloy
    },

    // 第8行：DN48~53
    {
        {6, 1.1296, 0x2900, 15, 10, 10000.0}, // plastic
        {6, 1.4694, 0x2A00, 15, 14, 10000.0}, // metal
        {6, 1.7326, 0x2A00, 7, 14, 10000.0}   // alloy
    }};

uint8_t inline GetMaterialConfigIndex()
{
    int i;
    double diameter = g_parameters.outer_diameter - 2 * g_parameters.wall_thickmess;
    for (i = 0; i < 9; i++)
    {
        if (diameter <= g_materialConfigs[i][0].max_diameter)
        {
            return i;
        }
    }
    return i;
}
e2prom_status_t SaveParameters(Parameters_t *params)
{
    e2prom_status_t ret = E2PROM_OK;
    ret = bsp_e2prom_write(PARAM_STORE_ADDR, (uint8_t *)params, sizeof(Parameters_t));
    if (ret != E2PROM_OK)
    {
        log_e("SaveParameters failed");
    }
    return ret;
}

e2prom_status_t LoadParameters(Parameters_t *params)
{
    e2prom_status_t ret = E2PROM_OK;
    ret = bsp_e2prom_read(PARAM_STORE_ADDR, (uint8_t *)params, sizeof(Parameters_t));
    if (ret != E2PROM_OK)
    {
        log_e("LoadParameters failed");
    }
    return ret;
}
