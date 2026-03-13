/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file task_start.c
 * 
 * @par dependencies 
 * - task_manager.h
 * @author rnz | Rvision
 * 
 * @brief Provide the task of start FreeRTOS tasks.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-10-23
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/
#include "task_manager.h"
#include "elog.h"
#include "main.h"

extern int Load$$RW_IRAM1$$Base;
static TaskHandle_t task_start_Handle = NULL;/* 创建任务句柄 */
static void task_start(void *params)
{
    // 创建任务
    taskENTER_CRITICAL();
    do_create_bsp_e2prom_task();
    do_create_bsp_gp22_task();
    do_create_bsp_uart_task();
    do_create_bsp_key_task();
    do_create_uart_parse_task();
    do_create_display_task();
    do_create_calculate_task(); 
    do_create_monitor_task();
    do_create_lvgl_task();
    taskEXIT_CRITICAL(); // 提前退出临界区
    vTaskDelete(task_start_Handle);
    //启动系统调度
    vTaskStartScheduler();
    
}
void do_create_start_task(void)
{	
	BaseType_t xReturn = pdPASS;						
	xReturn = xTaskCreate(task_start, "task_start", 64, NULL, 1, &task_start_Handle);;        
	if(pdPASS == xReturn)	vTaskStartScheduler();  	
}
