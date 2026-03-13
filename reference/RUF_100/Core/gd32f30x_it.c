/*!
    \file    gd32f30x_it.c
    \brief   interrupt service routines

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f30x_it.h"
#include "main.h"
#include "systick.h"
#include "FreeRTOS.h"
#include "bsp_key.h"
#include "bsp_gp22_handler.h"
#include "bsp_gp22_driver.h"
#include "bsp_uart_driver.h"
extern void xPortSysTickHandler(void);
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void SVC_Handler(void)
//{
//}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void PendSV_Handler(void)
//{
//}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    tick_add();
    #if (INCLUDE_xTaskGetSchedulerState  == 1 )
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        {
	#endif   
		xPortSysTickHandler();
	#if (INCLUDE_xTaskGetSchedulerState  == 1 )
			}
	#endif  

}

void TIMER6_IRQHandler(void) 
{
    // 检查是否为更新中断
    if (timer_interrupt_flag_get(TIMER6, TIMER_INT_FLAG_UP) != RESET) 
    {
        delay_decrement();                  // 递减延迟计数器
        timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);  // 清除中断标志
    }
}
void TIMER5_IRQHandler(void) 
{
    // 检查是否为更新中断
    if (timer_interrupt_flag_get(TIMER5, TIMER_INT_FLAG_UP) != RESET) 
    {
        delay_us_decrement();                  // 递减延迟计数器
        timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);  // 清除中断标志
    }
}

//every 4ms emit 
void TIMER1_IRQHandler(void) 
{
    // 检查是否为更新中断
    if (timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_UP) != RESET) 
    {
        loop_switch();
        gp22_start_meature_tof();
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);  // 清除中断标志
    }
}

void EXTI0_IRQHandler(void)
{

    if (RESET != exti_interrupt_flag_get(EXTI_0)) 
	{
	    exti_interrupt_flag_clear(EXTI_0);
        GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    }
}
void EXTI1_IRQHandler(void)
{
    if (RESET != exti_interrupt_flag_get(EXTI_1)) 
	{
	    exti_interrupt_flag_clear(EXTI_1);
        GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    }
}
void EXTI2_IRQHandler(void)
{
    if (RESET != exti_interrupt_flag_get(EXTI_2)) 
	{
	    exti_interrupt_flag_clear(EXTI_2);
        GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    }
}
void EXTI5_9_IRQHandler(void)
{
    if (RESET != exti_interrupt_flag_get(EXTI_6)) 
	{
	    exti_interrupt_flag_clear(EXTI_6);
        GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    }
}


//GPIO中断回调函数
void GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin) 
    {
        case KEY0_GPIO_PIN:
        case KEY1_GPIO_PIN:
        case KEY2_GPIO_PIN:
        case KEY3_GPIO_PIN:
            key_gpio_exti_handler(GPIO_Pin);
            break;
        default:
            return; 
    }
}

//串口接收回调
void DMA0_Channel4_IRQHandler(void)
{
    if(SET == dma_interrupt_flag_get(DMA0, DMA_CH4, DMA_INT_FLAG_FTF)) 
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH4, DMA_INT_FLAG_FTF);
        dma_comp_irq_callback(CIRCULAR_BUFFER_SIZE);
    }

    if(SET == dma_interrupt_flag_get(DMA0, DMA_CH4, DMA_INT_FLAG_HTF)) 
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH4, DMA_INT_FLAG_HTF);
        dma_half_irq_callback(CIRCULAR_BUFFER_SIZE/2);
    }
}

// USART0的中断响应函数
void USART0_IRQHandler(void)
{
	if(SET == usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
    {
        uint32_t remaning_bytes = dma_transfer_number_get(DMA0, DMA_CH4);
        uart_idle_irq_callback(CIRCULAR_BUFFER_SIZE - remaning_bytes);

        usart_data_receive(USART0);
	}
}