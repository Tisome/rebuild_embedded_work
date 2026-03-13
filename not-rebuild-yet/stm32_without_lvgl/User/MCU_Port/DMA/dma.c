/**
 * @file dma.c
 * @author Ye Dingzai (yedingzai@126.com)
 * @brief
 * @version 0.1
 * @date 2026-02-02
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "./SYSTEM/sys/sys.h"
#include "./MCU_Port/DMA/dma.h"

DMA_HandleTypeDef hdma_spi1_rx;

void vDMA1_SPI1_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma_spi1_rx.Instance = DMA1_Channel2;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_spi1_rx);

    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5, 0); /* 设置DMA1通道2中断优先级 */
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);         /* 使能DMA1通道2中断 */
}

void DMA1_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi1_rx);
}
