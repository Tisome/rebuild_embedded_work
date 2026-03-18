/************************* freertos header file *************************/
#include "FreeRTOS.h"
#include "semphr.h"

/************************* perph header file *************************/
#include "delay.h"
#include "exti.h"
#include "sys.h"

/************************* app header file *************************/
#include "freertos_resources.h"

void key_exti_init(void);
void FPGA_gpio_exti_init(void);
void nvic_init(void);

/**
 * @brief 外部中断初始化
 *
 */
void perph_exti_init(void)
{
    key_gpio_exti_init();
    FPGA_gpio_exti_init();
    nvic_init();
}

/* K1-K4外部中断GPIO初始化 */
void key_gpio_exti_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    /* 使能按键外部中断GPIO时钟 */
    KEY_1_INT_GPIO_CLK_ENABLE();
    KEY_2_INT_GPIO_CLK_ENABLE();
    KEY_3_INT_GPIO_CLK_ENABLE();
    KEY_4_INT_GPIO_CLK_ENABLE();

    gpio_init_struct.Pin = KEY_1_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING_FALLING;   /* 上升沿 + 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                   /* 上拉 */
    HAL_GPIO_Init(KEY_1_INT_GPIO_PORT, &gpio_init_struct); /* KEY_1配置为上升沿和下降沿触发中断 */

    gpio_init_struct.Pin = KEY_2_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING_FALLING;   /* 上升沿 + 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                   /* 上拉 */
    HAL_GPIO_Init(KEY_2_INT_GPIO_PORT, &gpio_init_struct); /* KEY_2配置为上升沿和下降沿触发中断 */

    gpio_init_struct.Pin = KEY_3_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING_FALLING;   /* 上升沿 + 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                   /* 上拉 */
    HAL_GPIO_Init(KEY_3_INT_GPIO_PORT, &gpio_init_struct); /* KEY_3配置为上升沿和下降沿触发中断 */

    gpio_init_struct.Pin = KEY_4_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING_FALLING;   /* 上升沿 + 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                   /* 上拉 */
    HAL_GPIO_Init(KEY_4_INT_GPIO_PORT, &gpio_init_struct); /* KEY_4配置为上升沿和下降沿触发中断 */
}

/* FPGA中断外部中断GPIO初始化 */
void FPGA_gpio_exti_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    FPGA_INT_GPIO_CLK_ENABLE();

    gpio_init_struct.Pin = FPGA_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;          /* 上升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                /* 下拉 */
    HAL_GPIO_Init(FPGA_INT_GPIO_PORT, &gpio_init_struct); /* FPGA_INT配置为上升沿触发中断 */
}

/* NVIC中断优先级设置和使能 */
void nvic_init(void)
{
    /* 设置按键外部中断优先级并使能 */
    HAL_NVIC_SetPriority(KEY_1_INT_IRQn, 13, 0); /* 设置K1按键的中断优先级 */
    HAL_NVIC_SetPriority(KEY_2_INT_IRQn, 12, 0); /* 设置K2按键的中断优先级 */
    HAL_NVIC_SetPriority(KEY_3_INT_IRQn, 11, 0); /* 设置K3按键的中断优先级 */
    HAL_NVIC_SetPriority(KEY_4_INT_IRQn, 10, 0); /* 设置K4按键的中断优先级 */

    HAL_NVIC_EnableIRQ(KEY_1_INT_IRQn); /* 使能K1按键中断 */
    HAL_NVIC_EnableIRQ(KEY_2_INT_IRQn); /* 使能K2按键中断 */
    HAL_NVIC_EnableIRQ(KEY_3_INT_IRQn); /* 使能K3按键中断 */
    HAL_NVIC_EnableIRQ(KEY_4_INT_IRQn); /* 使能K4按键中断 */

    /* 设置FPGA_INT中断优先级并使能 */
    HAL_NVIC_SetPriority(FPGA_INT_IRQn, 1, 0); /* 设置FPGA_INT中断优先级 */
    HAL_NVIC_EnableIRQ(FPGA_INT_IRQn);         /* 使能FPGA_INT中断 */

    /* 设置DMA1通道2中断优先级并使能 */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5, 0); /* 设置DMA1通道2中断优先级 */
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);         /* 使能DMA1通道2中断 */
}

/* K1外部中断服务函数 */
void KEY_1_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_1_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_1_INT_GPIO_PIN);
}

/* K2外部中断服务函数 */
void KEY_2_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_2_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_2_INT_GPIO_PIN);
}

/* K3外部中断服务函数 */
void KEY_3_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_3_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_3_INT_GPIO_PIN);
}

/* K4外部中断服务函数 */
void KEY_4_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_4_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_4_INT_GPIO_PIN);
}

/* FPGA_INT外部中断服务函数 */
void FPGA_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(FPGA_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(FPGA_INT_GPIO_PIN);
}

/* 外部中断回调服务函数 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    switch (GPIO_Pin)
    {
    case (KEY_1_INT_GPIO_PIN):
    case (KEY_2_INT_GPIO_PIN):
    case (KEY_3_INT_GPIO_PIN):
    case (KEY_4_INT_GPIO_PIN):
        key_gpio_exti_handler(GPIO_Pin);
        break;
    case (FPGA_INT_GPIO_PIN):
        fpga_gpio_exti_handler(GPIO_Pin);
        break;
    default:
        break;
    }
}

void fpga_gpio_exti_handler(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (GPIO_Pin == FPGA_INT_GPIO_PIN)
    {
        xSemaphoreGiveFromISR(xSem_FPGA_INT, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
