#include "rcu.h"
void rcu_config(void)
{

    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1); 
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2); 
    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1); 


    //开启相关时钟
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    
    rcu_periph_clock_enable(RCU_USART0);
    
    rcu_periph_clock_enable(RCU_SPI2);

    rcu_periph_clock_enable(RCU_SPI1);

    rcu_periph_clock_enable(RCU_I2C0);

    rcu_periph_clock_enable(RCU_DMA1);

    rcu_periph_clock_enable(RCU_DMA0);

    rcu_periph_clock_enable(RCU_DAC);
    
    rcu_periph_clock_enable(RCU_AF);

    rcu_periph_clock_enable(RCU_TIMER0);

    rcu_periph_clock_enable(RCU_TIMER1);

    rcu_periph_clock_enable(RCU_TIMER3);

    rcu_periph_clock_enable(RCU_TIMER4);

    rcu_periph_clock_enable(RCU_TIMER6);

    rcu_periph_clock_enable(RCU_TIMER5);
}