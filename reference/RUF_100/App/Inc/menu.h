/******************************************************************************
 * Copyright (C) 2025 Rvision.
 * 
 * All Rights Reserved.
 * 
 * @file menu.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
 * @author rnz
 * 
 * @brief Provide the HAL APIs of MENU and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-7-03
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

#ifndef _MENU_H
#define _MENU_H

//*********************** Includes *************************//

#include <stdint.h>
#include <stdio.h>

//*********************** Includes *************************//

//****************** Defines ************************//

#define DEFAULT_ITEMS_PER_PAGE 3 // 每页显示的默认最大项目数

// 菜单动作类型定义
typedef int (*MenuAction)(void*);

// 菜单项结构
typedef struct 
{
    //图片
    const char* text;     // 菜单项文本
    MenuAction action;    // 菜单项回调函数、
    const char* text_cn;     // 菜单项文本
} MenuItem;

// 菜单结构
typedef struct {
    const char* title[2];        // 菜单标题       // 菜单标题
    const uint8_t items_count;      // 菜单项数量
    const MenuItem* items;    // 菜单项数组
    int page;                 // 当前页码
    uint8_t items_per_page;   // 每页显示的项目数
    uint8_t start;   //最先开始显示的ietm 编号
    uint8_t type;   //判断是显示菜单还是设置菜单
} Menu;
//****************** Defines ************************//

// 显示模式定义
typedef enum {
    FLOW_RATE_MODE,
    HISTORICAL_FLOW_MODE,
    DISPLAY_MODE_COUNT  // 跟踪模式总数
} DisplayMode;

// 显示配置结构体
typedef struct {
    const char* title;
    double* value;
    const char* value_unit;
    double* secondary_value;
    const char* secondary_unit;
} DisplayConfig;

// 流量显示初始化
void flow_display_init(void);

// 主显示函数
void display_flow_screen(void);
// 公共接口：显示并处理菜单
int display_menu(Menu* menu);
int display_history_data(const char* title, int item_count, 
                         void(*display_item)(int index, int y_pos));

int menu3_history_bymonth(void);
int menu3_history_byday(void);
int menu3_history_byyear(void);
int menu3_history_total(void);

void flow_display_init(void);
// 主菜单处理
int main_menu_handler(void*);

int menu1_start_handler(void*);

int menu1_setting_handler(void*);

int menu1_history_handler(void*);

int menu1_display_rate_handler(void *);

//
int menu2_pipsetting_handler(void*);
int menu2_syssetting_handler(void*);
int menu2_calsetting_handler(void*);
int menu2_outsetting_handler(void*);
int menu2_unitsetting_handler(void *);
//

int menu2_history_byday_handler(void*);
int menu2_history_bymonth_handler(void*);
int menu2_history_byyear_handler(void*);
int menu2_history_total_handler(void*);
int menu2_displaysetting_handler(void *);
//
int menu3_outer_diameter_handler(void*);
int menu3_wall_thickmess_handler(void*);
int menu3_material_handler(void*);
int menu3_fluid_setting_handler(void*);
int menu3_display_direction_handler(void *);
int menu3_alarm_handler(void *);
//
int menu3_flow_rate_unit_handler(void*);
int menu3_total_unit_handler(void*);
int menu3_time_set_handler(void*);
int menu3_system_info_handler(void*);
//
int menu3_scale_factor_handler(void*);
int menu3_4_20_ma_handler(void*);
int menu3_set_zero_handler(void*);
//
int menu3_rs485_setting_handler(void*);
int menu3_4_20_mh_handler(void*);
int menu3_modbus_setting_handler(void*);
int menu3_npn_pnp_setting_handler(void*);
int menu3_output_mode_handler(void* parameter);

#endif