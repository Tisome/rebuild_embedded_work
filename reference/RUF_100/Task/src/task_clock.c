#include "task_manager.h"
#include "rtc.h"
#include "stdio.h"
#include "data.h"
#include "e2prom.h"
#include "elog.h"
#include <math.h>
#include "bsp_at24c512_handler.h"
#include "bsp_iwdg_handler.h"
extern Parameters_t g_parameters;
static TaskHandle_t task_clock_Handle = NULL;/* 创建任务句柄 */

RTC_TimeTypeDef g_currentTime;
RTC_DateTypeDef g_currentDate;

int g_offset_day = 0;
int g_offset_month = 0;
int g_offset_year = 0;

double g_flow_history;         //累计流量统计
double g_flow_every_day    = 0;//每天流量统计
double g_flow_every_month  = 0;//每月流量统计
double g_flow_every_year   = 0;//每年流量统计
double g_flow_info         = 0;
// 历史数据结构
double g_history_days[15] = {0,0,0,0,0,0,0,0,0,0,0,0};   // 最近12天每天的流量
double g_history_months[12] = {0,0,0,0,0,0,0,0,0,0,0,0}; // 最近12个月每月的流量
double g_history_years[2] = {0,0};   // 最近2年的年流量
extern RTC_HandleTypeDef hrtc; /* RTC控制句柄 */
void time_init();
void history_data_init();
void history_data_test();
static void task_clock(void *params)
{
    TaskHandle_t *oled_task_handler = get_task_oled_Handle();
    history_data_init();
    xTaskNotifyGive(*oled_task_handler);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(400); // 500ms周期
    
    
    uint8_t last_saved_min   = 0xFF;
    uint8_t last_saved_day   = 0xFF;
    uint8_t last_saved_month = 0xFF;
    uint8_t last_saved_year  = 0xFF;
    wdt_monitor_task_register("clock_task", 2000, &clock_task_id);
    wdt_monitor_task_enable(clock_task_id);
    while (1) 
    {
        HAL_RTC_GetTime(&hrtc, &g_currentTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &g_currentDate, RTC_FORMAT_BIN);
        RTC_Backup_DateTime();
        // 每秒0秒且分钟为5的倍数时保存流量（历史流量）
        if(g_currentTime.Seconds == 0 && ((g_currentTime.Minutes % 1) == 0)) 
        {
            if(last_saved_min != g_currentTime.Minutes) 
            {
                g_flow_history      += g_flow_info;
                g_flow_every_day    += g_flow_info;
                g_flow_every_month  += g_flow_info;
                g_flow_every_year   += g_flow_info;
                SaveFlow_default(&g_flow_history);
                SaveFlow_day(&g_flow_every_day);
                SaveFlow_month(&g_flow_every_month);
                SaveFlow_year(&g_flow_every_year);
                last_saved_min = g_currentTime.Minutes;
                vTaskDelay(100); 
            }
        }
        // 每秒0秒且分钟为0的,时钟为0时保存一天累计流量
        if(g_currentTime.Seconds == 0 && g_currentTime.Minutes == 0 && g_currentTime.Hours == 0) 
        {
            if(last_saved_day != g_currentDate.Date) 
            {
                g_flow_every_day = 0;
                last_saved_day = g_currentDate.Date;
                g_offset_day += 8;
                vTaskDelay(100); 
            }
        }
        // 每秒0秒且分钟为0，时钟为0，日期为1号时保存一个月流量
        if(g_currentTime.Seconds == 0 && g_currentTime.Minutes == 0 && g_currentTime.Hours == 0 &&g_currentDate.Date == 1) 
        {
            if(last_saved_month != g_currentDate.Month) 
            {
                g_flow_every_month = 0;
                last_saved_month = g_currentDate.Month;
                g_offset_month += 8;
                vTaskDelay(100); 
            }
        }
        // 每秒0秒且分钟为0，时钟为0，日期为1号，月份为1月时保存一年流量
        if(g_currentTime.Seconds == 0 && g_currentTime.Minutes == 0 && g_currentTime.Hours == 0 &&g_currentDate.Date == 1 && g_currentDate.Month == 1) 
        {
            if(last_saved_year != g_currentDate.Year) 
            {
                g_flow_every_year = 0;
                last_saved_year = g_currentDate.Year;
                g_offset_year += 8;
                vTaskDelay(100); 
            }
        }

        wdt_monitor_task_feed(clock_task_id);
        if(1!=g_parameters.is_saved) 
        {
            g_parameters.is_saved = 1;
        }
        SaveParameters(&g_parameters);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
TaskHandle_t* get_task_clock_Handle(void)
{
    return &task_clock_Handle;
}

void do_create_clock_task(void)
{	
	BaseType_t xReturn = pdPASS;						/* 定义一个创建信息返回值，默认为pdPASS */
	/* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate(task_clock, "task_clock", 256, NULL, 24, &task_clock_Handle);

}
void history_data_init()
{
    int month_days[12] = {31,29,31,30,31,30,31,31,30,31,30,31}; 
    int offset = 0;
     //设置保存地址偏移量
    g_offset_day = g_currentDate.Date * 8 - 8;
    for (int i = 0; i<g_currentDate.Month - 1; i++)
    {   
        g_offset_day += ( month_days[i] * 8 );
    }
    g_offset_month = g_currentDate.Month * 8 - 8;
    g_offset_year = g_currentDate.Year * 8 - 8;
//        log_d("offset year=%d,month=%d,day=%d",g_offset_day,g_offset_month,g_offset_year);
     //获取历史数据
    for(int i = 0; i < 15; i++ )
    {   
        offset = ((g_offset_day - i + 366) % 366) * 8;
        if (offset >= 0) 
        {
            LoadFlow_By_Day(&g_history_days[i] , offset);
            if(isnan(g_history_days[i]))  g_history_days[i - 1] = 0;
        }
    }
    for(int i = 0; i < 12; i++ )
    {   
        offset = ((g_offset_month - i + 12) % 12) * 8;
        if (offset >= 0) 
        {
            LoadFlow_By_Month(&g_history_months[i] , offset);
//            log_d("g_offset_month = %d value=%f\r\n",offset,g_history_months[i]);
            if(isnan(g_history_months[i])) g_history_months[i - 1] = 0;
        }
    }
    for(int i = 0; i < 2; i++ )
    {   
        offset = (g_offset_year - i) * 8;
        if (offset >= 0) 
        {   
            LoadFlow_By_Year(&g_history_years[i], offset);
            if(isnan(g_history_years[i])) g_history_years[i - 1] = 0;
        }
    }

    LoadFlow_default(&g_flow_history);
}

void history_data_test()
{
    int offset = 0;
    //保存测试历史数据
     //设置保存地址偏移量
    int month_days[12] = {31,29,31,30,31,30,31,31,30,31,30,31}; 
    g_offset_day = g_currentDate.Date * 8 - 8;
    for (int i = 0; i<g_currentDate.Month - 1; i++)
    {   
        g_offset_day += ( month_days[i] * 8 );
    }
    g_offset_month = g_currentDate.Month * 8 - 8;
    g_offset_year = g_currentDate.Year * 8 - 8;
//    log_d("offset year=%d,month=%d,day=%d",g_offset_day,g_offset_month,g_offset_year);
    double history_days[15] = {3,3,3,4,5,6,7,8,9,10,11,12,13,14,15};   // 最近15天每天的流量
    double history_months[12] = {5,5,5,4,5,6,7,8,9,10,11,12};    // 最近12个月每月的流量
    double history_years[2] = {2000,2000}; 
    for(int i = 0; i <15; i++ )
    {   
        offset = ((g_offset_day - i + 366) % 366) * 8;
        SaveFlow_By_Day(&history_days[i] , offset);
//                                 vTaskDelay(pdMS_TO_TICKS(100)); // 适当延时
    }
    for(int i = 0; i < 12; i++ )
    {   
        offset = ((g_offset_month - i + 12) % 12) * 8;
//        log_d("g_offset_month = %d value=%f\r\n",offset,history_months[i]);
        SaveFlow_By_Month(&history_months[i] , offset);
//                                 vTaskDelay(pdMS_TO_TICKS(100)); // 适当延时
    }
        for(int i = 0; i < 2; i++ )
    {   
        offset = (g_offset_year - i) * 8; 
        SaveFlow_By_Year(&history_years[i] , offset);
//                                 vTaskDelay(pdMS_TO_TICKS(100)); // 适当延时
    }
}
