
#ifndef __task_manager_h__
#define __task_manager_h__
//创建相关同步互斥工具
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//#include "cmsis_os.h"

//用于看门狗线程的线程id
//extern uint32_t clock_task_id;
extern uint32_t calculate_task_id;
//extern uint32_t uart_task_id;

// APP tasks
void do_create_start_task(void);

void do_create_display_task(void);   
TaskHandle_t* get_task_display_Handle(void);


void do_create_lvgl_task(void);   
TaskHandle_t* get_task_lvgl_Handle(void);


void do_create_calculate_task(void);   
TaskHandle_t* get_task_calculate_Handle(void);

void do_create_uart_parse_task(void);

// BSP Tasks
void do_create_bsp_key_task(void);
TaskHandle_t* get_bsp_key_task_Handle(void);

TaskHandle_t* get_bsp_uart_task_Handle(void);
void do_create_bsp_uart_task(void);


TaskHandle_t* get_bsp_e2prom_task_Handle(void);
void do_create_bsp_e2prom_task(void);

TaskHandle_t* get_bsp_gp22_task_Handle(void);
void do_create_bsp_gp22_task(void);


// system tasks
TaskHandle_t* get_task_monitor_Handle(void);
void do_create_monitor_task(void);
#endif
