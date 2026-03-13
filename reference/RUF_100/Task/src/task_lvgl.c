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
static TaskHandle_t task_lvgl_Handle = NULL;/* 创建任务句柄 */

static void task_lvgl(void *params)
{
	// st7789_instance_driver_link(&st7789_driver_instance, &st7789_oper_instance, &st7789_spi_instance ,&st7789_time_instance, &st7789_yield);  
  	// st7789_driver_instance.pf_st7789_init(&st7789_driver_instance);
	// lv_port_disp_init();
	// lv_port_indev_init();
  	// time3_tft_bl_init();
  	// time3_tft_bl_set(0.5);   
	// //lvgl init
	// ui_menu_list_init();
	// ui_menu_list_set("MAIN MENU");
	// for(int i = 0;i<3;i++)
	// {
	// 	ui_menu_list_item_set(i,"test",1,0);
	// }
    while(1)
	{
		// int key = key_scan(0);
		// if(key == KEY2_PRES)
		// {
		// 	lv_obj_t* label = lv_label_create(screen);
		// 	lv_label_set_text(label, "Hello, LVGL!");
		// 	lv_obj_align(label, LV_ALIGN_CENTER, 0, 40);  // 将标签居中对齐
		// }
		__disable_irq();
		lv_timer_handler(); /* let the GUI do its work */
		__enable_irq();
		vTaskDelay(10);
		// st7789_driver_instance.pf_fill(&st7789_driver_instance,10,50,110,150,WHITE);
    }
} 
TaskHandle_t* get_task_lvgl_Handle(void)
{
    return &task_lvgl_Handle;
}
void do_create_lvgl_task(void)
{	
	xTaskCreate(task_lvgl, "task_lvgl", 512 * 4, NULL, 11, &task_lvgl_Handle);
    
}
