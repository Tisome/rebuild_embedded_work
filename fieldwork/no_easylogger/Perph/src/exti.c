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
void FPGA_INT_exti_init(void);

/**
 * @brief 外部中断初始化
 *
 */
void perph_exti_init(void)
{
    key_exti_init();
    FPGA_INT_exti_init();
}

/**
 * @brief 按键外部中断初始化
 *
 */
void key_exti_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    /* 使能按键外部中断GPIO时钟 */
    KEY_RIGHT_INT_GPIO_CLK_ENABLE();
    KEY_LEFT_INT_GPIO_CLK_ENABLE();
    KEY_UP_INT_GPIO_CLK_ENABLE();
    KEY_DOWN_INT_GPIO_CLK_ENABLE();

    gpio_init_struct.Pin = KEY_UP_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;            /* 上升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                  /* 下拉 */
    HAL_GPIO_Init(KEY_UP_INT_GPIO_PORT, &gpio_init_struct); /* KEY_UP配置为下降沿触发中断 */

    gpio_init_struct.Pin = KEY_DOWN_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;             /* 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                      /* 上拉 */
    HAL_GPIO_Init(KEY_DOWN_INT_GPIO_PORT, &gpio_init_struct); /* KEY_DOWN配置为下降沿触发中断 */

    gpio_init_struct.Pin = KEY_LEFT_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;             /* 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                      /* 上拉 */
    HAL_GPIO_Init(KEY_LEFT_INT_GPIO_PORT, &gpio_init_struct); /* KEY_LEFT配置为下降沿触发中断 */

    gpio_init_struct.Pin = KEY_RIGHT_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;              /* 下降沿触发 */
    gpio_init_struct.Pull = GPIO_PULLUP;                       /* 上拉 */
    HAL_GPIO_Init(KEY_RIGHT_INT_GPIO_PORT, &gpio_init_struct); /* KEY_RIGHT配置为下降沿触发中断 */

    HAL_NVIC_SetPriority(KEY_UP_INT_IRQn, 13, 0);    /* 设置向上按键的中断优先级 */
    HAL_NVIC_SetPriority(KEY_DOWN_INT_IRQn, 12, 0);  /* 设置向下按键的中断优先级 */
    HAL_NVIC_SetPriority(KEY_LEFT_INT_IRQn, 11, 0);  /* 设置向左按键的中断优先级 */
    HAL_NVIC_SetPriority(KEY_RIGHT_INT_IRQn, 10, 0); /* 设置向右按键的中断优先级 */

    HAL_NVIC_EnableIRQ(KEY_UP_INT_IRQn);    /* 使能向上按键中断 */
    HAL_NVIC_EnableIRQ(KEY_DOWN_INT_IRQn);  /* 使能向下按键中断 */
    HAL_NVIC_EnableIRQ(KEY_LEFT_INT_IRQn);  /* 使能向左按键中断 */
    HAL_NVIC_EnableIRQ(KEY_RIGHT_INT_IRQn); /* 使能向右按键中断 */
}

/**
 * @brief FPGA中断外部中断初始化
 *
 */
void FPGA_INT_exti_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    FPGA_INT_GPIO_CLK_ENABLE();

    gpio_init_struct.Pin = FPGA_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_IT_RISING;          /* 上升沿触发 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                /* 下拉 */
    HAL_GPIO_Init(FPGA_INT_GPIO_PORT, &gpio_init_struct); /* FPGA_INT配置为上升沿触发中断 */

    HAL_NVIC_SetPriority(FPGA_INT_IRQn, 1, 0); /* 设置FPGA_INT中断优先级 */
    HAL_NVIC_EnableIRQ(FPGA_INT_IRQn);         /* 使能FPGA_INT中断 */
}

/**
 * @brief 上键外部中断服务函数
 *
 */
void KEY_UP_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_UP_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_UP_INT_GPIO_PIN);
}

/**
 * @brief 下键外部中断服务函数
 *
 */
void KEY_DOWN_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_DOWN_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_DOWN_INT_GPIO_PIN);
}

/**
 * @brief 左键外部中断服务函数
 *
 */
void KEY_LEFT_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_LEFT_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_LEFT_INT_GPIO_PIN);
}

/**
 * @brief 右键外部中断服务函数
 *
 */
void KEY_RIGHT_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_RIGHT_INT_GPIO_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_RIGHT_INT_GPIO_PIN);
}

/**
 * @brief 外部中断回调函数
 *
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (GPIO_Pin != FPGA_INT_GPIO_PIN)
    {
        sKeyFlag = GPIO_Pin; /* 记录按下的按键GPIO引脚 */
        xSemaphoreGiveFromISR(xSem_key_Filter, &xHigherPriorityTaskWoken);
    }
    else if (GPIO_Pin == FPGA_INT_GPIO_PIN)
    {
        xSemaphoreGiveFromISR(xSem_FPGA_INT, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
