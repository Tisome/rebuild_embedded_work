/**
 * @file RCT6_EXTI.c
 * @author Ye Dingzai (yedingzai@126.com)
 * @brief
 * @version 0.1
 * @date 2026-02-12
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <stdio.h>
#include "gd32f30x.h"
#include "RCT6_EXTI.h"
#include "RCT6_led.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define SW1_KEY_PIN GPIO_PIN_8
#define SW1_KEY_GPIO_PORT GPIOA
#define SW1_KEY_GPIO_CLK RCU_GPIOA
#define SW1_KEY_EXTI_LINE EXTI_8
#define SW1_KEY_EXTI_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define SW1_KEY_EXTI_PIN_SOURCE GPIO_PIN_SOURCE_8
#define SW1_KEY_EXTI_IRQn EXTI5_9_IRQn

extern SemaphoreHandle_t xSem_ISR;

void KEY_SW1_EXTI_Init(void)
{
    rcu_periph_clock_enable(SW1_KEY_GPIO_CLK);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(SW1_KEY_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,
              SW1_KEY_PIN);

    nvic_irq_enable(SW1_KEY_EXTI_IRQn, 6U, 0U);
    gpio_exti_source_select(SW1_KEY_EXTI_PORT_SOURCE, SW1_KEY_EXTI_PIN_SOURCE);

    exti_init(SW1_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(SW1_KEY_EXTI_LINE);
}

void KEY_SW1_EXTI_Callback(void)
{
    LED_TOGGLE();
    printf("key SW1 EXTI test!\r\n");
}

void EXTI5_9_IRQHandler(void)
{
    if (exti_interrupt_flag_get(SW1_KEY_EXTI_LINE))
    {
        exti_interrupt_flag_clear(SW1_KEY_EXTI_LINE);
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        /* 通知SPI_Rx任务当前数据已经接收完成 */
        xSemaphoreGiveFromISR(xSem_ISR, &xHigherPriorityTaskWoken);
        /* 强制任务切换 */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        //KEY_SW1_EXTI_Callback();
        
    }
}

/* FPGA中断外部中断相关宏定义 */
// RCT6上就用PA1吧，到时候和FPGA连接一下试试看

#define FPGA_INT_GPIO_PORT GPIOA
#define FPGA_INT_GPIO_PIN GPIO_PIN_1
#define FPGA_INT_GPIO_CLK RCU_GPIOA
#define FPGA_INT_EXTI_LINE EXTI_1
#define FPGA_INT_EXTI_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define FPGA_INT_EXTI_PIN_SOURCE GPIO_PIN_SOURCE_1
#define FPGA_INT_GPIO_IRQn EXTI1_IRQn
#define FPGA_INT_IRQHandler EXTI1_IRQHandler

void FPGA_INT_EXTI_Init(void)
{
    rcu_periph_clock_enable(FPGA_INT_GPIO_CLK);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(FPGA_INT_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,
              FPGA_INT_GPIO_PIN);
    nvic_irq_enable(FPGA_INT_GPIO_IRQn, 2U, 0U);
    gpio_exti_source_select(FPGA_INT_EXTI_PORT_SOURCE, FPGA_INT_EXTI_PIN_SOURCE);

    exti_init(FPGA_INT_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(FPGA_INT_EXTI_LINE);
}

// 在这里是给freeRTOS的信号量，等移植好再说
void EXTI1_IRQHandler(void)
{
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (exti_interrupt_flag_get(FPGA_INT_EXTI_LINE))
    {
        exti_interrupt_flag_clear(FPGA_INT_EXTI_LINE);
        // xSemaphoreGiveFromISR(xSem_FPGA_INT, &xHigherPriorityTaskWoken);
    }
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
