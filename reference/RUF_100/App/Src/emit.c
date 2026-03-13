#include "emit.h"
#include "elog.h"
#include "main.h"

uint32_t freq_list[FREQ_COUNT] = {2000000, 2000000, 2000000, 2000000, 2000000}; 

#define EMIT_LOOP_1 0
#define EMIT_LOOP_2 1

volatile  uint8_t current_freq_idx = 0;          
volatile  uint32_t current_pulse_cnt = 0;           
void emit_init(void) 
{
    timer_deinit(TIMER4);
    timer_ic_parameter_struct timer_icinitpara;
    timer_parameter_struct timer_initpara;

    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler         = 12 - 1;                 
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.period            = (10000000 / freq_list[0]) - 1; 
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1; 
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER4, &timer_initpara);


    timer_oc_parameter_struct timer_ocinitpara;
    timer_channel_output_struct_para_init(&timer_ocinitpara);

    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE; 
    timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_LOW;
    timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_LOW;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER4, TIMER_CH_3, &timer_ocinitpara);

    timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_3, timer_initpara.period / 2);
    timer_channel_output_mode_config(TIMER4, TIMER_CH_3, TIMER_OC_MODE_PWM1); 
    timer_channel_output_shadow_config(TIMER4,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);

    timer_auto_reload_shadow_enable(TIMER4);

    timer_disable(TIMER4);

    current_freq_idx = 0;       
    current_pulse_cnt = 0;   
}


void pwm_set_freq(uint32_t freq) 
{
    uint32_t period = (120000000 / freq) - 1; 
    timer_autoreload_value_config(TIMER4, period);

    timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_3, period / 2);

    timer_counter_value_config(TIMER4, period / 2);
}


void sweep_start(void) 
{     
    timer_enable(TIMER4);  
    for(int i =0;i<100;i++) __NOP();
    timer_counter_value_config(TIMER4,0);
    timer_disable(TIMER4); 
}


void loop_switch()
{
    static uint8_t loop_chose = EMIT_LOOP_1;
    if(loop_chose == EMIT_LOOP_1)
    {
        gpio_bit_reset(SIG_SEL_GPIO_Port,SIG_SEL_Pin);
        gpio_bit_reset(FIRE_SEL_GPIO_Port,FIRE_SEL_Pin);
        loop_chose = EMIT_LOOP_2;
    }
    else if(loop_chose == EMIT_LOOP_2)
    {   
        gpio_bit_set(SIG_SEL_GPIO_Port,SIG_SEL_Pin);
        gpio_bit_set(FIRE_SEL_GPIO_Port,FIRE_SEL_Pin);
        loop_chose =EMIT_LOOP_1;
    }
}

