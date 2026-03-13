#include "RCT6_USART.h"
#include "gd32f30x.h"
#include <stdio.h>

void USART0_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    usart_deinit(USART0);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_baudrate_set(USART0, 115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

    //    nvic_irq_enable(USART1_IRQn, 1, 1);
    //    usart_interrupt_enable(USART1, USART_INT_RBNE);
    //    usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);

    usart_enable(USART0);
}

void USART0_SendChar(uint8_t c)
{
    while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
        ;
    usart_data_transmit(USART0, c);
    while (usart_flag_get(USART0, USART_FLAG_TC) == RESET)
        ;
}
