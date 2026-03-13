#include "gpio.h"


void gpio_config()
{   
    //disable jtag, enable swd
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    gpio_pin_remap_config(GPIO_TIMER0_PARTIAL_REMAP, ENABLE);
    /* configure TFT GPIO port */ 
    gpio_init(TFT_DC_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TFT_DC_Pin);
    gpio_init(TFT_RESET_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TFT_RESET_Pin);
    gpio_init(TFT_BL_GPIO_Port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, TFT_BL_Pin);
    
    gpio_init(TFT_SCL_GPIO_Port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, TFT_SCL_Pin);
    gpio_init(TFT_SDA_GPIO_Port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, TFT_SDA_Pin);
    gpio_init(TFT_CS_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TFT_CS_Pin);
    /* configure TFT GPIO port */ 

    /* configure KEY GPIO port */
    gpio_init(KEY0_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY0_GPIO_PIN); 
    gpio_init(KEY1_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY1_GPIO_PIN); 
    gpio_init(KEY2_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY2_GPIO_PIN); 
    gpio_init(KEY3_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY3_GPIO_PIN); 

    nvic_irq_enable(EXTI0_IRQn, 5U, 0U);
    nvic_irq_enable(EXTI1_IRQn, 5U, 0U);
    nvic_irq_enable(EXTI2_IRQn, 5U, 0U);
    nvic_irq_enable(EXTI5_9_IRQn, 5U, 0U);


    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_0);
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_1);
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_2);
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_6);

    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_init(EXTI_1, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_init(EXTI_6, EXTI_INTERRUPT, EXTI_TRIG_BOTH);

    exti_interrupt_flag_clear(EXTI_0);
    exti_interrupt_flag_clear(EXTI_1);
    exti_interrupt_flag_clear(EXTI_2);
    exti_interrupt_flag_clear(EXTI_6);
    /* configure KEY GPIO port */


    /* configure E2PROM GPIO port */
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, EEPROM_I2C_SDA_PIN);
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, EEPROM_I2C_SCL_PIN);

    gpio_bit_set(EEPROM_I2C_GPIO_PORT,EEPROM_I2C_SDA_PIN);
    gpio_bit_set(EEPROM_I2C_GPIO_PORT,EEPROM_I2C_SCL_PIN);
    
    /* configure GP22 GPIO port */    
    gpio_init(GP22_SCL_GPIO_Port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GP22_SCL_Pin);
    gpio_init(GP22_MISO_GPIO_Port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GP22_MISO_Pin);
    gpio_init(GP22_MOSI_GPIO_Port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GP22_MOSI_Pin);
    gpio_init(GP22_CS_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GP22_CS_Pin);

    gpio_init(GP22_START_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GP22_START_Pin);

    /* configure DAC GPIO port */ 
    gpio_init(DAC_COM_GPIO_Port, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, DAC_COM_Pin);
    gpio_init(DAC_FIRE_GPIO_Port, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, DAC_FIRE_Pin);


    /* configure TIMER4 CH3 GPIO port */ 
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* configure TIMER0 CH0/CH0_ON GPIO port */ 
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

    /* configure FIRE Switch GPIO port */ 
    gpio_init(FIRE_SEL_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FIRE_SEL_Pin); 
    gpio_init(SIG_SEL_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SIG_SEL_Pin);

    /* configure FLOW_OUT Select GPIO port */
    gpio_init(FLOW_OUT_SEL_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FLOW_OUT_SEL_Pin);

    /* configure USART0 GPIO port */
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    /* configure RS485 Control GPIO port */
    gpio_init(RS485_CONTROL_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RS485_CONTROL_Pin);
    gpio_bit_reset(RS485_CONTROL_GPIO_Port,RS485_CONTROL_Pin);
}
