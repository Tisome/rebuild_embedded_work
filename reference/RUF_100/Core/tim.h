#ifndef __TIM_H__
#define __TIM_H__
#include "gd32f30x.h"


void tim6_delay_init(void);
void tim5_delay_init(void);
void time1_4ms_delay_init(void); 
void time3_tft_bl_init(void); 
void time3_tft_bl_set(double channel_duty_cycle);  
#endif
