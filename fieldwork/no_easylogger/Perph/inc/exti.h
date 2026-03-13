#ifndef __EXTI_H
#define __EXTI_H

#include "sys.h"

/* 向右按键外部中断相关宏定义 */
#define KEY_RIGHT_INT_GPIO_PORT GPIOE
#define KEY_RIGHT_INT_GPIO_PIN  GPIO_PIN_4
#define KEY_RIGHT_INT_GPIO_CLK_ENABLE() \
    do {                                \
        __HAL_RCC_GPIOE_CLK_ENABLE();   \
    } while (0) /* PE口时钟使能 */
#define KEY_RIGHT_INT_IRQn       EXTI4_IRQn
#define KEY_RIGHT_INT_IRQHandler EXTI4_IRQHandler

/* 向下按键外部中断相关宏定义 */
#define KEY_DOWN_INT_GPIO_PORT GPIOE
#define KEY_DOWN_INT_GPIO_PIN  GPIO_PIN_3
#define KEY_DOWN_INT_GPIO_CLK_ENABLE() \
    do {                               \
        __HAL_RCC_GPIOE_CLK_ENABLE();  \
    } while (0) /* PE口时钟使能 */
#define KEY_DOWN_INT_IRQn       EXTI3_IRQn
#define KEY_DOWN_INT_IRQHandler EXTI3_IRQHandler

/* 向左按键外部中断相关宏定义 */
#define KEY_LEFT_INT_GPIO_PORT GPIOE
#define KEY_LEFT_INT_GPIO_PIN  GPIO_PIN_2
#define KEY_LEFT_INT_GPIO_CLK_ENABLE() \
    do {                               \
        __HAL_RCC_GPIOE_CLK_ENABLE();  \
    } while (0) /* PE口时钟使能 */
#define KEY_LEFT_INT_IRQn       EXTI2_IRQn
#define KEY_LEFT_INT_IRQHandler EXTI2_IRQHandler

/* 向上按键外部中断相关宏定义 */
#define KEY_UP_INT_GPIO_PORT GPIOA
#define KEY_UP_INT_GPIO_PIN  GPIO_PIN_0
#define KEY_UP_INT_GPIO_CLK_ENABLE()  \
    do {                              \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
    } while (0) /* PA口时钟使能 */
#define KEY_UP_INT_IRQn       EXTI0_IRQn
#define KEY_UP_INT_IRQHandler EXTI0_IRQHandler

/* FPGA中断外部中断相关宏定义 */
#define FPGA_INT_GPIO_PORT GPIOA
#define FPGA_INT_GPIO_PIN  GPIO_PIN_11
#define FPGA_INT_GPIO_CLK_ENABLE()    \
    do {                              \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
    } while (0) /* PA口时钟使能 */
#define FPGA_INT_IRQn       EXTI15_10_IRQn
#define FPGA_INT_IRQHandler EXTI15_10_IRQHandler

/**
 * @brief RUF-X外部中断初始化
 *
 */
void perph_exti_init(void);

#endif
