#ifndef _PARAMETER_EDITOR_H
#define _PARAMETER_EDITOR_H

#include <stdint.h>
// 参数编辑类型
typedef struct {
    const char* name;     // 参数名称
    const char* unit;     // 参数单位
    double* value;        // 参数值指针
    uint8_t precision;    // 显示精度
    uint8_t digits;       // 总位数
} Parameter;

// 参数选择类型
typedef struct {
    const char* name;        // 参数标题
    const uint8_t items_count;      // 菜单项数量
    const char* items[10];    // 菜单项数组
    int page;                 // 当前页码
    uint8_t items_per_page;   // 每页显示的项目数
} Parameter_select;

int menu3_outer_diameterSetting(void);
int menu3_wall_thickmessSetting(void);
int menu3_scale_factorSetting(void);
int menu3_zero_cutSetting(void);
int menu4_modbus_addr_handler(void * parameter);
int menu4_lower_range_handler(void * parameter);
int menu4_upper_range_handler(void * parameter);
int menu4_alarm_upper_range_handler(void * parameter);
int menu4_alarm_lower_range_handler(void * parameter);
int menu4_pulse_per_flowSetting(void * parameter);
#endif