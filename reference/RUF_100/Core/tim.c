#include "tim.h"
#include "data.h"

#define TFT_BL_FREQ_HZ      300
//none os tick
void tim6_delay_init(void) 
{

    // 复位TIM6，确保初始状态
    timer_deinit(TIMER6);

    //  配置TIM6基本参数
    timer_parameter_struct tim_init_struct;
    timer_struct_para_init(&tim_init_struct);  // 初始化结构体默认值

    tim_init_struct.prescaler         = 120 - 1;         // 预分频：120MHz/(119+1)=1MHz（1μs计数）
    tim_init_struct.counterdirection  = TIMER_COUNTER_UP;  // 向上计数
    tim_init_struct.period            = 999;        // 自动重装载值：1MHz×1000=1ms中断
    tim_init_struct.clockdivision     = TIMER_CKDIV_DIV1;  // 时钟不分频
    tim_init_struct.repetitioncounter = 0;          // 基本定时器无重复计数，设0
    timer_init(TIMER6, &tim_init_struct);

    // 清除更新中断标志（避免初始化后立即触发中断）
    timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);

    // 使能TIM6更新中断（计数器溢出时触发）
    timer_interrupt_enable(TIMER6, TIMER_INT_UP);

    // 配置NVIC中断优先级（优先级5，高于FreeRTOS的SysTick）
    nvic_irq_enable(TIMER6_IRQn, 5, 0);  // 抢占优先级5，子优先级0

    // 启动TIM6计数器
    timer_enable(TIMER6);
}

void tim5_delay_init(void) 
{

    // 复位TIM6，确保初始状态
    timer_deinit(TIMER5);

    //  配置TIM6基本参数
    timer_parameter_struct tim_init_struct;
    timer_struct_para_init(&tim_init_struct);  // 初始化结构体默认值

    tim_init_struct.prescaler         = 12 - 1;         // 预分频：120MHz/(19+1)=10MHz（1μs计数）
    tim_init_struct.counterdirection  = TIMER_COUNTER_UP;  // 向上计数
    tim_init_struct.period            = 9;        // 自动重装载值：10MHz×10=1us中断
    tim_init_struct.clockdivision     = TIMER_CKDIV_DIV1;  // 时钟不分频
    tim_init_struct.repetitioncounter = 0;          // 基本定时器无重复计数，设0
    timer_init(TIMER5, &tim_init_struct);

    // 清除更新中断标志（避免初始化后立即触发中断）
    timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);

    // 使能TIM6更新中断（计数器溢出时触发）
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);

    // 配置NVIC中断优先级（优先级5，高于FreeRTOS的SysTick）
    nvic_irq_enable(TIMER5_IRQn, 5, 0);  // 抢占优先级5，子优先级0

    // 启动TIM6计数器
    timer_enable(TIMER5);
}

void time1_4ms_delay_init(void) 
{    
    timer_deinit(TIMER1);

    timer_parameter_struct tim_init_struct;
    timer_struct_para_init(&tim_init_struct);  

    tim_init_struct.prescaler         = 480 - 1;         
    tim_init_struct.counterdirection  = TIMER_COUNTER_UP; 
    tim_init_struct.period            = 999;
    tim_init_struct.clockdivision     = TIMER_CKDIV_DIV1;  
    tim_init_struct.repetitioncounter = 0;         
    timer_init(TIMER1, &tim_init_struct);

    timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);

    timer_interrupt_enable(TIMER1, TIMER_INT_UP);

    nvic_irq_enable(TIMER1_IRQn, 1, 0);  

    timer_enable(TIMER1);

}


void time3_tft_bl_init(void) 
{    
    timer_deinit(TIMER3);

    timer_parameter_struct timer_initpara;
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 120 -1;                 
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.period            = (1000000 / TFT_BL_FREQ_HZ) - 1; 
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1; 
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER3, &timer_initpara);


    timer_oc_parameter_struct timer_ocinitpara;
    timer_channel_output_struct_para_init(&timer_ocinitpara);
    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE; 
    timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH; 
    timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW; 
    timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER3, TIMER_CH_3, &timer_ocinitpara);

    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_3, timer_initpara.period / 2);
    timer_channel_output_mode_config(TIMER3, TIMER_CH_3, TIMER_OC_MODE_PWM0); 
    timer_channel_output_shadow_config(TIMER3,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(TIMER3);

    
    timer_enable(TIMER3);
 

}

void time3_tft_bl_set(double channel_duty_cycle) 
{

    int duty_cycle = ((1000000 / TFT_BL_FREQ_HZ) - 1) * channel_duty_cycle;
    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_3, duty_cycle);

    timer_counter_value_config(TIMER3, duty_cycle);
}

