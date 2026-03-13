#include "gd32f30x.h"
#include "RCT6_led.h"

void LED_Init(void)
{
    rcu_periph_clock_enable(LED_CLK_GPIO_PORT);                                // 开启GPIOA时钟
    gpio_init(LED_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_GPIO_PIN); // PB0配置为推挽输出
}


