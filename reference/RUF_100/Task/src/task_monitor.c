#include "task_manager.h"
#include "bsp_iwdg_handler.h"

//要监控线程的id
uint32_t calculate_task_id;

static TaskHandle_t task_monitor_Handle = NULL;/* 创建任务句柄 */
static void task_monitor(void *params)
{

    TickType_t xTicksToWait = pdMS_TO_TICKS(WDT_MONITOR_WAIT_MS);
    wdt_monitor_init();
    while (1)
    {
        if (IWDT_SUCCESS != monitor_task_timeout_check())
        {
            system_hang_up();
        }
        else
        {
            bsp_iwdt_feed();
        }

        ulTaskNotifyTake(pdTRUE, xTicksToWait);
    }
}
TaskHandle_t* get_task_monitor_Handle(void)
{
    return &task_monitor_Handle;
}

void do_create_monitor_task(void)
{	
	BaseType_t xReturn = pdPASS;						/* 定义一个创建信息返回值，默认为pdPASS */
	/* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate(task_monitor, "task_monitor", 64, NULL, 30, &task_monitor_Handle);;        
}
