#include "elog.h"
#include "task_manager.h"

static TaskHandle_t task_start_handle = NULL; /* 创建任务句柄 */
static void task_start(void *params)
{
    // taskENTER_CRITICAL();

    //  输出
    do_create_display_task(); // DONE

    // 计算
    do_create_algorithm_task(); // DONE

    // 按键输入
    do_create_bsp_key_task(); // DONE

    // spi输出
    do_create_spi_rx_task(); // DONE

    // e2prom
    do_create_bsp_e2prom_task(); // DONE

    // uart接受数据
    do_create_bsp_uart_task(); // DONE

    // modbus解析
    do_create_modbus_parse_task(); // DONE

    // modbus执行
    do_create_modbus_execute_task(); // DONE

    // watchdog
    do_create_watchdog_task(); // DONE

    // 测试数据
    do_create_test_task(); // DONE

    // taskEXIT_CRITICAL(); // 提前退出临界区

    vTaskDelete(NULL); // 删除自身任务
}

void do_create_start_task(void)
{
    BaseType_t xReturn = pdPASS;
    xReturn = xTaskCreate(task_start, "task_start", 128, NULL, 1, &task_start_handle);
    if (xReturn != pdPASS)
    {
        log_e("create task_start failed\r\n");
    }
}