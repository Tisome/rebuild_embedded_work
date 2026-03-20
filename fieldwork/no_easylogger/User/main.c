/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.4
 * @date        2022-01-04
 * @brief       FreeRTOS ʵ��
 * @license     Copyright (c) 2020-2032, �������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "delay.h"
#include "rs485.h"
#include "spi.h"
#include "sys.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

#include "elog.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9); /*  72Mhz */

    delay_init(72);

    usart_init(115200);

    my_elog_init();

    spi1_dma_init();
    uart1_dma_init();
    rs485_init();

    freertos_resources_init();

    log_i("hello world\n");

    do_create_start_task();

    vTaskStartScheduler();

    while (1)
    {
    }
}
