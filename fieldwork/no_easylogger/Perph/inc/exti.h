#ifndef __EXTI_H
#define __EXTI_H

#include "sys.h"

/************************* 按键中断 *************************/

/* K1按键外部中断相关宏定义 */
/* K1按键连接到PC0引脚，使用EXTI0中断线 */
#define KEY_1_INT_GPIO_PORT GPIOC
#define KEY_1_INT_GPIO_PIN  GPIO_PIN_0
#define KEY_1_INT_GPIO_CLK_ENABLE()   \
    do {                              \
        __HAL_RCC_GPIOC_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */
#define KEY_1_INT_IRQn       EXTI0_IRQn
#define KEY_1_INT_IRQHandler EXTI0_IRQHandler

/* K2按键外部中断相关宏定义 */
/* K2按键连接到PC1引脚，使用EXTI1中断线 */
#define KEY_2_INT_GPIO_PORT GPIOC
#define KEY_2_INT_GPIO_PIN  GPIO_PIN_1
#define KEY_2_INT_GPIO_CLK_ENABLE()   \
    do {                              \
        __HAL_RCC_GPIOC_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */
#define KEY_2_INT_IRQn       EXTI1_IRQn
#define KEY_2_INT_IRQHandler EXTI1_IRQHandler

/* K3按键外部中断相关宏定义 */
/* K3按键连接到PC2引脚，使用EXTI2中断线 */
#define KEY_3_INT_GPIO_PORT GPIOC
#define KEY_3_INT_GPIO_PIN  GPIO_PIN_2
#define KEY_3_INT_GPIO_CLK_ENABLE()   \
    do {                              \
        __HAL_RCC_GPIOC_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */
#define KEY_3_INT_IRQn       EXTI2_IRQn
#define KEY_3_INT_IRQHandler EXTI2_IRQHandler

/* K4按键外部中断相关宏定义 */
/* K4按键连接到PC3引脚，使用EXTI3中断线 */
#define KEY_4_INT_GPIO_PORT GPIOC
#define KEY_4_INT_GPIO_PIN  GPIO_PIN_3
#define KEY_4_INT_GPIO_CLK_ENABLE()   \
    do {                              \
        __HAL_RCC_GPIOC_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */
#define KEY_4_INT_IRQn       EXTI3_IRQn
#define KEY_4_INT_IRQHandler EXTI3_IRQHandler

/************************* FPGA中断 *************************/

/* FPGA中断外部中断相关宏定义 */
/* FPGA中断连接到PA11引脚，使用EXTI11中断线 */
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
