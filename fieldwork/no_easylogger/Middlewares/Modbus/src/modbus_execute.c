#include "modbus_frame_process.h"
#include "modbus_map.h"

#include "data.h"

#include "FreeRTOS.h"

void task_modbus_execute(void *parameter)
{
    modbus_cmd_t cmd;

    while (1)
    {
        if (xQueueReceive(g_modbus_cmd_queue, &cmd, portMAX_DELAY) == pdPASS)
        {
            switch (cmd)
            {
            case MODBUS_CMD_CLEAR_TOTALIZER:
                g_algo_state.q_total_m3 = 0.0;
                g_algo_out.flow_rate_total = 0.0;
                break;

            case MODBUS_CMD_ZERO_LEARN_START:
                /* 启动零漂学习 */
                break;

            case MODBUS_CMD_SAVE_PARAMETERS:
                /* 保存参数到 flash */
                SaveParameters(&g_parameters);
                break;

            case MODBUS_CMD_LOAD_DEFAULT_PARAMETERS:
                parameter_init();
                break;

            case MODBUS_CMD_CLEAR_ALARM:
                g_alarm = ALARM_OK;
                break;

            case MODBUS_CMD_SOFT_RESET:
                /* 软件复位 */
                // 先保存一下参数
                SaveParameters(&g_parameters);
                NVIC_SystemReset();
                break;

            default:
                break;
            }
        }
    }
}
