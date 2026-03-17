/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.4
 * @date        2022-01-04
 * @brief       FreeRTOS 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "delay.h"
#include "sys.h"
#include "usart.h"

// #include "./BSP/LED/led.h"
// #include "./BSP/LCD/lcd.h"
// #include "./BSP/KEY/key.h"
// #include "./BSP/SRAM/sram.h"
// #include "./MALLOC/malloc.h"
// #include "freertos_demo.h"
#include "FreeRTOS.h"
#include "freertos_resources.h"
#include "task.h"

#include "elog.h"

int main(void)
{
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */

    delay_init(72);
    /* 延时初始化 */ // 包括了设置systick给freertos用

    usart_init(115200); /* 串口初始化为115200 */
    // led_init();                         /* 初始化LED */
    // lcd_init();                         /* 初始化LCD */
    // key_init();                         /* 初始化按键 */

    my_elog_init();

    freertos_resources_init();

    log_i("hello world\n");

    do_create_start_task();

    vTaskStartScheduler(); /* 启动调度器 */

    while (1)
    {
    }
}
