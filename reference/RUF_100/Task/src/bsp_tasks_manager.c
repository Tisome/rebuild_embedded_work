#include "task_manager.h"
#include "elog.h"
#include "bsp_key.h"
#include "link_total.h"
#include "bsp_uart_driver.h"

static TaskHandle_t bsp_task_key_handle         = NULL;/* 创建任务句柄 */
static TaskHandle_t bsp_gp22_handler            = NULL;/* 创建任务句柄 */
static TaskHandle_t bsp_e2prom_handler          = NULL;/* 创建任务句柄 */
static TaskHandle_t bsp_uart_handler            = NULL;/* 创建任务句柄 */

TaskHandle_t* get_bsp_key_task_Handle(void)
{
    return &bsp_task_key_handle;
}
void do_create_bsp_key_task(void)
{	
	BaseType_t xReturn = pdPASS;					

	xReturn = xTaskCreate(key_task_func, "bsp_task_key", 128, 
		                    NULL, 27, &bsp_task_key_handle);  
	if(xReturn != pdPASS)
	{
		log_e("Failed to create bsp_key_task!");
	}
}

TaskHandle_t* get_bsp_gp22_task_Handle(void)
{
    return &bsp_gp22_handler;
}

void do_create_bsp_gp22_task(void)
{	
	BaseType_t xReturn = pdPASS;						

	xReturn = xTaskCreate(gp22_handler_thread, "bsp_task_gp22", 256, 
		            (void *)&gp22_input_arg, 31, &bsp_gp22_handler);  

	if(xReturn != pdPASS)
	{
		log_e("Failed to create bsp_key_task!");
	}
}


TaskHandle_t* get_bsp_e2prom_task_Handle(void)
{
    return &bsp_e2prom_handler;
}
void do_create_bsp_e2prom_task(void)
{	
	BaseType_t xReturn = pdPASS;					
	xReturn = xTaskCreate(e2prom_handler_thread, "bsp_task_e2prom", 512, 
			        (void *)&e2prom_input_arg,29, &bsp_e2prom_handler);  
	if(xReturn != pdPASS)
	{
		log_e("Failed to create bsp_e2prom_task!");
	}
}

TaskHandle_t* get_bsp_uart_task_Handle(void)
{
    return &bsp_uart_handler;
}
void do_create_bsp_uart_task(void)
{	
	BaseType_t xReturn = pdPASS;						

	xReturn = xTaskCreate(uart_driver_func, "bsp_task_uart", 256, 
		                            NULL, 30, &bsp_uart_handler);  
}
