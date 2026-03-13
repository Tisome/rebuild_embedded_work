#include "task_manager.h"
#include "main.h"
#include "fonts.h"
#include"link_total.h"
#include "tim.h"
#include "data.h"
static TaskHandle_t task_display_Handle = NULL;/* 创建任务句柄 */
extern uint8_t start_pip;//启动界面修正


static void task_display(void *params)
{ 
  int ret;
  st7789_instance_driver_link(&st7789_driver_instance, &st7789_oper_instance, &st7789_spi_instance ,&st7789_time_instance, &st7789_yield);  
  st7789_driver_instance.pf_st7789_init(&st7789_driver_instance);
	lv_port_disp_init();
  time3_tft_bl_init();
  time3_tft_bl_set(0.5);   
	//lvgl init
  ui_menu_total_init();
  ui_diplay_init();
  ui_display_all_disappear();
  ui_edit_init();
  ui_edit_all_disappear();
	ui_menu_list_init();
  st7789_rotate(&st7789_driver_instance,g_parameters.rotate_state);
  while(1)
	{
    vTaskDelay(20);
    ret = main_menu_handler(NULL); 
    if(ret == 0) start_pip = 0;
  }
}
TaskHandle_t* get_task_display_Handle(void)
{
    return &task_display_Handle;
}
void do_create_display_task(void)
{	
	xTaskCreate(task_display, "task_display", 512 * 2, NULL, 13, &task_display_Handle);
    
}