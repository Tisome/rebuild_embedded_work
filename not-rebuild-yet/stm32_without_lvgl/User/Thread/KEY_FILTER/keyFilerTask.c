/**
 * @file keyFilerTask.c
 * @author Ye Dingzai (yedingzai@126.com)
 * @brief
 * @version 0.1
 * @date 2026-02-01
 *
 * @copyright Copyright (c) 2026
 *
 * 用于实现按键滤波的任务
 *
 */

#include "./Thread/KEY_FILTER/keyFilterTask.h"
#include "./Thread/DEMO/RUF_X_demo.h"
#include "./MCU_Port/EXTI/exti.h"

void vKeyFilter_task(void *pvParameter)
{
    uint16_t key_pin = 0;
    while (1)
    {
        if (xSemaphoreTake(xSem_key_Filter, portMAX_DELAY) == pdTRUE)
        {
            key_pin = sKeyFlag;
            vTaskDelay(10); /* 获取按下的按键GPIO引脚 */
            switch (key_pin)
            {
            case KEY_UP_INT_GPIO_PIN:
            {
                /* 处理上键按下事件 */
                break;
            }
            case KEY_DOWN_INT_GPIO_PIN:
            {
                /* 处理下键按下事件 */
                break;
            }
            case KEY_LEFT_INT_GPIO_PIN:
            {
                /* 处理左键按下事件 */
                break;
            }
            case KEY_RIGHT_INT_GPIO_PIN:
            {
                /* 处理右键按下事件 */
                break;
            }
            default:
                break;
            }
        }
    }
}



