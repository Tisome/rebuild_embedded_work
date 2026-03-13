#include "app_config.h"

#if (RCT6 || CGT6)

#include "gd32f30x.h"
#include "usart.h"
#include <stdio.h>

#define USART0_TX_GPIO_PORT GPIOA
#define USART0_TX_GPIO_PIN GPIO_PIN_9

#define USART0_RX_GPIO_PORT GPIOA
#define USART0_RX_GPIO_PIN GPIO_PIN_10

void USART0_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);
    // 设置usart tx的gpio初始化 输出
    gpio_init(USART0_TX_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, USART0_TX_GPIO_PIN);
    // 设置usart rx的gpio初始化 读入
    gpio_init(USART0_RX_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, USART0_RX_GPIO_PIN);

    usart_deinit(USART0);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_baudrate_set(USART0, 115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

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

#endif
