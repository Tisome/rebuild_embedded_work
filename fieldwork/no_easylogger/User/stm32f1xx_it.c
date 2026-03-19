/**
 ******************************************************************************
 * @file    Templates/Src/stm32f1xx.c
 * @author  MCD Application Team
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_it.h"
#include "circular_buffer.h"
#include "spi.h"
#include "stm32f1xx_hal.h"
#include "sys.h"
#include "usart.h"

/** @addtogroup STM32F1xx_HAL_Examples
 * @{
 */

/** @addtogroup Templates
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
#if (!SYS_SUPPORT_OS)
void SVC_Handler(void)
{
}
#endif

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
#if (!SYS_SUPPORT_OS)
void PendSV_Handler(void)
{
}
#endif

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
#if (!SYS_SUPPORT_OS)
void SysTick_Handler(void)
{
    HAL_IncTick();
}
#endif

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f1xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */

// dma1通道2中断服务函数,用于SPI1 RX DMA
void DMA1_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi1_rx);
}

/* USART1中断服务函数
 * 作用：
 * 1. 处理USART的IDLE空闲中断
 * 2. 把DMA当前写指针位置同步给软件层
 * 3. 再交给HAL处理其他USART相关中断
 */
void USART1_IRQHandler(void)
{
    /* 判断是否发生了IDLE中断,也即一帧数据结束 */
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)
    {
        /* 清除IDLE标志，不清除的话，会重复进入IDLE中断 */
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);

        /* 计算DMA当前写到缓冲区的哪个位置 */
        // 比如说，我从125/126/127/0/1/2/3/4，这几个地址上的数据都被本次写了
        // 那么DMA当前的写指针就停在了地址5上，而我们需要把这个位置告诉软件层
        // __HAL_DMA_GET_COUNTER(huart1.hdmarx)是表示DMA还剩多少个数据没有写入缓冲区的
        // 也就是128-5=123个数据还没有写入缓冲区，所以DMA当前写指针的位置就是128-123=5
        uint16_t remaining = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
        uint16_t pos = CIRCULAR_BUF_SIZE - remaining;

        /* 把DMA当前写指针位置传给驱动层 */
        uart1_idle_irq_callback(pos);
    }

    /* 继续交给HAL处理USART的其他中断源 */
    HAL_UART_IRQHandler(&huart1);
}

void DMA1_Channel5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart1_rx);
}

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
