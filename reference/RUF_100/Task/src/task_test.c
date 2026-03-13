#include "task_manager.h"
#include "main.h"
#include "fonts.h"
#include "link_total.h"
#include "tim.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_demo_widgets.h"
#include "lv_port_indev_template.h"
#include "st7789_reg_define.h"
#include "bsp_st7789_driver.h"
#include "bsp_key.h"
static TaskHandle_t task_test_Handle = NULL;/* 创建任务句柄 */

static void task_test(void *params)
{
	st7789_instance_driver_link(&st7789_driver_instance, &st7789_oper_instance, &st7789_spi_instance ,&st7789_time_instance, &st7789_yield);  
  	st7789_driver_instance.pf_st7789_init(&st7789_driver_instance);
	lv_port_disp_init();
	lv_port_indev_init();
  	time3_tft_bl_init();
  	time3_tft_bl_set(0.5);   
	ui_menu_list_init();

    while(1)
	{
		lv_timer_handler(); /* let the GUI do its work */
		vTaskDelay(10);
    }
} 
TaskHandle_t* get_task_test_Handle(void)
{
    return &task_test_Handle;
}
void do_create_test_task(void)
{	
	xTaskCreate(task_test, "task_test", 512 * 4, NULL, 13, &task_test_Handle);
    
}
