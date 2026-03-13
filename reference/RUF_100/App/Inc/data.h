#ifndef __DATA_H
#define __DATA_H
#include <stdint.h>
#include <string.h>
#include "bsp_at24c512_handler.h"

// 流体类型
typedef enum 
{
    LIQUID_WATER         = 0,     
    LIQUID_OIL          = 1,      
} liquid_t;


// 管道类型枚举
typedef enum 
{
    PIPE_PLATIC         = 0,      //PVC管道
    PIPE_METAL          = 1,      //钢管管道
    PIPE_ALLOY          = 2      //铜管管道
} PipeType;

typedef struct 
{
    uint8_t  year;          // 年
    uint8_t  month;         // 月
    uint8_t  day;           // 日
    uint8_t  hour;          // 时
    uint8_t  minute;        // 分
    uint8_t  second;        // 秒
} Timer_t;

typedef struct {
    // 所有double类型成员 (8字节对齐)
    double    outer_diameter; // 内径（单位根据需求）
    double    wall_thickmess; // 外径
    double    scale_factor;   //比例参数
    double    cos;            //cos值
    double    sin;
    double    lower_range;    //流量计算上限
    double    upper_range;    //流量计算下限
    double    alarm_lower_range;    //流量计算上限
    double    alarm_upper_range;    //流量计算下限
    double    modbus_addr;
    double    k_factor; //脉冲当量
    double    offset;
    double    zero_cut;
    
    // 所有int类型成员 (4字节对齐)
    int       is_saved;       //是否已经保存
    int       flow_rate_unit; //流速单位
    int       total_unit;     //流量单位
    int       output_mode;    //输出模式 0为4~20ma 1为PNP 2为NPN
    int       b_select_mode;
    int       rotate_state;
    int       sensitivity;
    float  fixed_error_bias;
    
    // 所有枚举和uint8_t类型成员
    PipeType  pipe_type;      // 管道类型
    uint8_t   fluid_type;     //流体类型

    //bool类型成员 (1字节对齐)
    bool      language;
} Parameters_t;

extern Parameters_t g_parameters;

#define TIMER_STORE_ADDR 0x1000 // 时间参数存储起始地址
#define PARAM_STORE_ADDR 0x0000 // 管道参数存储起始地址

// 定义结构体存储不同材料的配置
typedef struct {
    double      time_del;
    double      e_factor;
    uint16_t        mask;
    uint8_t     dac_fire;
    int         emit_num;
    double  max_diameter;
} MaterialConfig;

// 配置查找表
extern  MaterialConfig g_materialConfigs[9][3];

// 保存参数到EEPROM
e2prom_status_t SaveParameters(Parameters_t *params);

// 从EEPROM读取参数
e2prom_status_t LoadParameters(Parameters_t *params);
uint8_t GetMaterialConfigIndex(); 
#endif