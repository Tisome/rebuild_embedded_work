#include "parameter_editor.h"


// parameter_editor.c
#include "menu.h"
#include "data.h"
#include "bsp_key.h"
#include "st7789_reg_define.h"
#include "bsp_st7789_driver.h"
#include "bsp_gp22_handler.h"
#include "ctype.h"
#include "dac.h"
#include "lv_ui.h"

extern bsp_st7789_driver_t st7789_driver_instance;
extern Parameters_t g_parameters;

// 辅助数学函数：计算整数倍率 (10^n)
static int32_t get_scale_factor(uint8_t precision) {
    int32_t factor = 1;
    for(int i=0; i<precision; i++) factor *= 10;
    return factor;
}
static int edit_parameter(Parameter* param) {
    ui_menu_all_disappear();
    ui_edit_all_appear();
    ui_edit_open(param);
    static uint8_t move_id;
    while(1) 
    {
        static uint8_t KeyNum;
        KeyNum = key_scan(1);
        switch(KeyNum) 
        {
            case KEY2_PRES: 
                if(move_id < param->digits - 1)
                {
                    lv_spinbox_step_prev(g_lv_ui_edit->spinbox);
                    move_id ++;
                }
                else
                {
                    lv_spinbox_set_cursor_pos(g_lv_ui_edit->spinbox, 0);
                    move_id = 0;
                }
                break;
                
            case KEY0_PRES: 
                lv_spinbox_increment(g_lv_ui_edit->spinbox);
                break;
                
            case KEY1_PRES: 
                lv_spinbox_decrement(g_lv_ui_edit->spinbox);
                if(lv_spinbox_get_value(g_lv_ui_edit->spinbox) <= 0)
                {
                    lv_spinbox_set_value(g_lv_ui_edit->spinbox, 0);
                }
                break;
                
            case KEY3_PRES: 
                // *param->value = atof(buffer);
                if (param) 
                {
                    int32_t final_int = lv_spinbox_get_value(g_lv_ui_edit->spinbox);
                    // 还原回 double
                    *param->value = (double)final_int / get_scale_factor(param->precision);
                }
            
                // 删除窗口，清理全局指针
                ui_edit_all_disappear();
                ui_menu_all_appear();
                return 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


// 外层直径设置
int menu3_outer_diameterSetting(void) {
    Parameter param = {
        .name = "Outer diameter",
        .unit = "mm",
        .value = &(g_parameters.outer_diameter) ,
        .precision = 2,
        .digits = 5
    };
    
    int result = edit_parameter(&param);
    
    if (g_parameters.wall_thickmess > g_parameters.outer_diameter)
    g_parameters.outer_diameter = g_parameters.wall_thickmess + 0.01;
    if (result == 0) 
    {
        __disable_irq();
//        DAC_SET_FIRE(g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].dac_fire);
        gp22_app_modify_reg_attr(GP22_CONF_REG2, g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
                                                                 g_parameters.pipe_type);
        __enable_irq();
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }	
    return result;
}

// 管壁厚度设置
int menu3_wall_thickmessSetting(void) {
    Parameter param = {
        .name = "Wall thickmess",
        .unit = "mm",
        .value = &(g_parameters.wall_thickmess) ,
        .precision = 2,
        .digits = 4
    };
    
    int result = edit_parameter(&param);
    if (g_parameters.wall_thickmess > g_parameters.outer_diameter)
    g_parameters.wall_thickmess = g_parameters.outer_diameter - 0.01;
    if (result == 0) {
        __disable_irq();
        dac_set_fir_vol(g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].dac_fire);
        gp22_app_modify_reg_attr(GP22_CONF_REG2, g_materialConfigs[GetMaterialConfigIndex()][g_parameters.pipe_type].mask,
                                                                 g_parameters.pipe_type);
        __enable_irq();
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
        
    }	
    return result;
}
// 比例因子设置
int menu3_scale_factorSetting(void) {
    Parameter param = {
        .name = "Scale factor",
        .unit = "",
        .value = &(g_parameters.scale_factor) ,
        .precision = 2,
        .digits = 3
    };
    
    int result = edit_parameter(&param);
    
    if (result == 0) {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }
    
    return result;
}

//modbus 地址设置
int menu4_modbus_addr_handler(void * parameter) {
    Parameter param = {
        .name = "Modbus addr",
        .unit = "",
        .value = &(g_parameters.modbus_addr) ,
        .precision = 0,
        .digits = 2
    };
    
    int result = edit_parameter(&param);
    
    if (result == 0) {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }
    
    return result;
}

// 4~20ma下限
int menu4_lower_range_handler(void * parameter) {
        Parameter param = {
        .name = "Lower range",
        .unit = "l/h",
        .value = &(g_parameters.lower_range) ,
        .precision = 1,
        .digits = 5
    };
    switch (g_parameters.flow_rate_unit)
    {
        case 0:
            param.unit="m3/h";
            break;
        case 1:
            param.unit="L/min";
            break;
        case 2:
            param.unit="mL/min";
            break;
        case 3:
            param.unit="L/h";
            break;
        default:
            param.unit="m3/h";
            break;
    }

    
    int result = edit_parameter(&param);
    if (g_parameters.lower_range >g_parameters.upper_range)
    g_parameters.lower_range = g_parameters.upper_range - 0.01;
    if (result == 0) {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }
    
    return result;
}
// 4~20ma上限
int menu4_alarm_upper_range_handler(void * parameter) {
        Parameter param = {
        .name = "Upper range",
        .unit = "l/h",
        .value = &(g_parameters.alarm_upper_range) ,
        .precision = 1,
        .digits = 5
    };
    switch (g_parameters.flow_rate_unit)
    {
        case 0:
            param.unit="m3/h";
            break;
        case 1:
            param.unit="L/min";
            break;
        case 2:
            param.unit="mL/min";
            break;
        case 3:
            param.unit="L/h";
            break;
        default:
            param.unit="m3/h";
            break;
    }

    
    int result = edit_parameter(&param);
    if (g_parameters.lower_range > g_parameters.upper_range)
    g_parameters.upper_range = g_parameters.lower_range + 0.01;
    if (result == 0) 
    {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }
    
    return result;
}

// 报警下限
int menu4_alarm_lower_range_handler(void * parameter) {
        Parameter param = {
        .name = "Lower range",
        .unit = "l/h",
        .value = &(g_parameters.alarm_lower_range) ,
        .precision = 1,
        .digits = 5
    };
    switch (g_parameters.flow_rate_unit)
    {
        case 0:
            param.unit="m3/h";
            break;
        case 1:
            param.unit="L/min";
            break;
        case 2:
            param.unit="mL/min";
            break;
        case 3:
            param.unit="L/h";
            break;
        default:
            param.unit="m3/h";
            break;
    }

    
    int result = edit_parameter(&param);
    if (g_parameters.alarm_lower_range >g_parameters.alarm_upper_range)
    g_parameters.alarm_lower_range = g_parameters.alarm_upper_range - 0.01;
    if (result == 0) {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }
    
    return result;
}
// 报警上限
int menu4_upper_range_handler(void * parameter) {
        Parameter param = {
        .name = "Upper range",
        .unit = "l/h",
        .value = &(g_parameters.alarm_upper_range) ,
        .precision = 1,
        .digits = 5
    };
    switch (g_parameters.flow_rate_unit)
    {
        case 0:
            param.unit="m3/h";
            break;
        case 1:
            param.unit="L/min";
            break;
        case 2:
            param.unit="mL/min";
            break;
        case 3:
            param.unit="L/h";
            break;
        default:
            param.unit="m3/h";
            break;
    }

    
    int result = edit_parameter(&param);
    if (g_parameters.alarm_lower_range > g_parameters.alarm_upper_range)
    g_parameters.alarm_upper_range = g_parameters.alarm_lower_range + 0.01;
    if (result == 0) 
    {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }
    
    return result;
}

int menu4_pulse_per_flowSetting(void * parameter)
{
    Parameter param = {
    .name = "Pulse K-factor",
    .unit = "",
    .value = &(g_parameters.k_factor) ,
    .precision = 2,
    .digits = 6
    };
    switch (g_parameters.total_unit)
    {
        case 0:
            param.unit="m3/P";
            break;
        case 1:
            param.unit="L/P";
            break;
        case 2:
            param.unit="mL/P";
            break;
        default:
            param.unit="m3/P";
            break;
    }

    
    int result = edit_parameter(&param);
    
    if (result == 0) {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }
    
    return result;
}

// 零切下限
int menu3_zero_cutSetting(void) {
    Parameter param = {
        .name = "Zero cut",
        .unit = "l/h",
        .value = &(g_parameters.zero_cut) ,
        .precision = 2,
        .digits = 5
    };
        switch (g_parameters.flow_rate_unit)
    {
        case 0:
            param.unit="m3/h";
            break;
        case 1:
            param.unit="L/min";
            break;
        case 2:
            param.unit="mL/min";
            break;
        case 3:
            param.unit="L/h";
            break;
        default:
            param.unit="m3/h";
            break;
    }
    
    int result = edit_parameter(&param);
    
    if (result == 0) 
    {
        g_parameters.is_saved = 1;
        SaveParameters(&g_parameters);
    }	
    return result;
}

