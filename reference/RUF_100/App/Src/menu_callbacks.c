#include "menu.h"
#include "data.h"
#include "bsp_key.h"
#include "menu_callbacks.h"
#include "math.h"


extern double g_flow_info;          // 上电启动至今的累计流量
// 历史流量数据
extern double g_history_days[15];   // 最近12天每天的流量
extern double g_history_months[12]; // 最近12个月每月的流量
extern double g_history_years[2];   // 最近2年的年流量
extern double empty;//占空比数值
//年月日数组
char * g_month[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

// 显示历史数据（支持分页）
static int display_history_data(const char* title, int item_count, 
                         void(*display_item)(int index, int y_pos)) {
    int page = 0;
    const int items_per_page = DEFAULT_ITEMS_PER_PAGE;
    int total_pages = (item_count + items_per_page - 1) / items_per_page;
    
    while(1) {
        // 清除菜单项区域（y坐标16px起，共 items_per_page 行）
        for (uint8_t i = 0; i < items_per_page; i++) {
            uint8_t ypos = 16 + i * 12;
            OLED_ShowString(0, ypos, (uint8_t*)"                     ", 12); // 21个空格清除整行
        }
        OLED_ShowString(2, 0, (uint8_t*)title, 12);
        
        // 显示当前页的数据
        int start_index = page * items_per_page;
        int end_index = start_index + items_per_page;
        if (end_index > item_count) end_index = item_count;
        
        for (int i = start_index; i < end_index; i++) {
            display_item(i, 16 + (i - start_index) * 12);
        }
        
        // 显示页码
        if (total_pages > 1) {
            char page_str[16];
            snprintf(page_str, sizeof(page_str), "Page %d/%d", page+1, total_pages);
            OLED_ShowString(60, 50, (uint8_t*)page_str, 12);
        }
        
        oled_refresh_gram();
        static uint8_t KeyNum;
        KeyNum = key_scan(0);
        if (KeyNum == KEY0_PRES) { // 上一页
            page = (page > 0) ? page - 1 : total_pages - 1;
        } else if (KeyNum == KEY1_PRES) { // 下一页
            page = (page + 1) % total_pages;
        } else if (KeyNum == KEY3_PRES) { // 返回
            return 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
                         
// 按天显示历史数据
static void display_day_item(int index, int y_pos) {
    // int month_days[12] = {31,29,31,30,31,30,31,31,30,31,30,31}; 
    // int day_idx = g_currentDate.Date - index - 1;
    // if (day_idx <= 0)
    // {
    //     day_idx += month_days[(g_currentDate.Month +10) % 12];
    //     char day_str[30];
    //     sprintf(day_str, "%s.%2d: %.2f m3", 
    //         g_month[g_currentDate.Month-2], 
    //         day_idx, 
    //         fabs(g_history_days[index]));
    //     OLED_ShowString(2, y_pos, (uint8_t*)day_str, 12);
    // }
    // else
    // {
    //     char day_str[30];
    //     sprintf(day_str, "%s.%2d: %.2f m3", 
    //         g_month[g_currentDate.Month-1], 
    //         day_idx, 
    //         g_history_days[index]);
    //     OLED_ShowString(2, y_pos, (uint8_t*)day_str, 12);
    // }
}

int menu3_history_byday(void) {
    return display_history_data("Day History Flow", 12, display_day_item);
}

// 按月显示历史数据
static void display_month_item(int index, int y_pos) {
    // int month_idx = (12 + g_currentDate.Month - index - 1) % 12;
    // char month_str[30];
    // sprintf(month_str, "%s: %.2f m3", 
    //         g_month[month_idx], 
    //         fabs(g_history_months[index]));
    // OLED_ShowString(2, y_pos, (uint8_t*)month_str, 12);
}

int menu3_history_bymonth(void) {
    return display_history_data("Month History Flow", 12, display_month_item);
}

// 按年显示历史数据
static void display_year_item(int index, int y_pos) {
    // int year_idx = 2000 + g_currentDate.Year - index - 1;
    // char year_str[30];
    // sprintf(year_str, "%d %s: %.2f m3", 
    //         year_idx,"Year",
    //         fabs(g_history_years[index]));
    // OLED_ShowString(2, y_pos, (uint8_t*)year_str, 12);
}

int menu3_history_byyear(void) {
    return display_history_data("Year History Flow", 2, display_year_item);
}
int menu3_history_total(void) {
    // while(1) {
    //     OLED_ShowString(2, 0, "Total Flow", 16);
        
    //     char total_str[30];
    //     sprintf(total_str, "%.2f m3", g_flow_info);
    //     OLED_ShowString(30, 24, (uint8_t*)total_str, 24);
        
    //     oled_refresh_gram();
    //     // 按键处理
    //     static uint8_t KeyNum;
    //     KeyNum = key_scan(0);
    //     if(KeyNum == KEY3_PRES) {  // 返回
    //         OLED_Clear();
    //         return 0;
    //     }
        
    //     vTaskDelay(pdMS_TO_TICKS(20));
    // }
    // return 0;
}

