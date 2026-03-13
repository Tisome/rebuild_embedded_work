/**
 * @file spi.c
 * @author Ye Dingzai (yedingzai@126.com)
 * @brief
 * @version 0.1
 * @date 2026-02-02
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "./SYSTEM/sys/sys.h"
#include "./MCU_Port/SPI/spi.h"
#include "./MCU_Port/DMA/dma.h"
#include "./Thread/DEMO/RUF_X_demo.h"

SPI_HandleTypeDef hspi1 = {0};
extern DMA_HandleTypeDef hdma_spi1_rx;

/**
 * @brief       SPI设备初始化代码
 * @note      主机模式,16位数据,硬件可片选,MASTER, CPOL = 0, CPHA = 0
 * @param       无
 * @retval      无
 */
void vSPI1_init(void)
{
    vDMA1_SPI1_Init(); /* 初始化SPI1的DMA接收 */

    SPI1_SPI_CLK_ENABLE(); /* 使能SPI1时钟 */

    hspi1.Instance = SPI1_SPI;                   /* SPI1 */
    hspi1.Init.Mode = SPI_MODE_MASTER;           /* 主机模式 */
    hspi1.Init.Direction = SPI_DIRECTION_2LINES; /* 2线全双工 */
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;     /* 8位数据帧格式 */
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;   /* 时钟悬空低电平 CPOL = 0 */
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       /* 第1个时钟沿采样数据 CPHA = 0 */

    hspi1.Init.NSS = SPI_NSS_SOFT; /* 软件管理NSS信号 */

    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; /* 波特率预分频16 */
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;                  /* 数据传输从MSB位开始 */
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;                  /* 关闭TI模式 */
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;  /* 关闭CRC计算 */
    hspi1.Init.CRCPolynomial = 7;                            /* CRC多项式 */

    __HAL_LINKDMA(&hspi1, hdmarx, hdma_spi1_rx); /* 关联DMA句柄 */

    HAL_SPI_Init(&hspi1); /* 初始化 */

    __HAL_SPI_ENABLE(&hspi1); /* 使能SPI外设 */

    SPI1_NSS_CS(FPGA_CS_DISABLE); /* 初始化后立刻确保CS为高电平 禁用 */
}

/**
 * @brief       SPI底层硬件管脚初始化函数
 * @note
 * @param hspi SPI句柄
 * @retval      无
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI1)
    {
        /* SPI1 GPIO配置 */
        SPI1_SCK_GPIO_CLK_ENABLE();
        SPI1_MISO_GPIO_CLK_ENABLE();
        SPI1_MOSI_GPIO_CLK_ENABLE();

        /* SPI1 SCK */
        GPIO_InitStruct.Pin = SPI1_SCK_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStruct);

        /* SPI1 MISO */
        GPIO_InitStruct.Pin = SPI1_MISO_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(SPI1_MISO_GPIO_PORT, &GPIO_InitStruct);

        /* SPI1 MOSI */
        GPIO_InitStruct.Pin = SPI1_MOSI_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStruct);

        /* SPI1 NSS */
        GPIO_InitStruct.Pin = SPI1_NSS_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(SPI1_NSS_GPIO_PORT, &GPIO_InitStruct);
    }
}

/**
 * @brief       设置SPI1波特率
 * @note      通过设置SPI1的CR1寄存器中的BR位来改变波特率
 * @param speed 波特率预分频值
 */
void vSPI1_set_speed(uint8_t speed)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(speed)); /* 判断有效性 */
    __HAL_SPI_DISABLE(&hspi1);                      /* 关闭SPI外设 */
    hspi1.Instance->CR1 &= 0XFFC7;                  /* 清除原来的波特率设置 */
    hspi1.Instance->CR1 |= speed << 3;              /* 设置新的波 */
    __HAL_SPI_ENABLE(&hspi1);                       /* 使能SPI外设 */
}

/**
 * @brief       SPI接收完成回调函数
 * @note
 * @param hspi SPI句柄
 * @retval      无
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        /* 通知SPI_Rx任务当前数据已经接收完成 */
        xSemaphoreGiveFromISR(xSem_SPI_Rx_Done, &xHigherPriorityTaskWoken);
        /* 强制任务切换 */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// /**
//  * @brief       SPI读取一个16位字
//  * @note
//  * @param       无
//  * @retval      读取到的数据
//  */
// uint16_t xSPI1_read_halfword(void)
// {
//     uint16_t rxdata;
//     HAL_SPI_Receive(&hspi1, &rxdata, 1, 500);
//     return rxdata;
// }

// /**
//  * @brief       SPI发送一个16位字
//  * @note
//  * @param txdata 要发送的数据
//  * @retval      无
//  */
// void vSPI1_write_word(uint16_t txdata)
// {
//     HAL_SPI_Transmit(&hspi1, (uint8_t *)&txdata, 1, 500);
// }

// /**
//  * @brief       SPI读写一个16位字
//  * @note
//  * @param txdata 要发送的数据
//  * @retval      读取到的数据
//  */
// uint16_t xSPI1_read_write_halfword(uint16_t txdata)
// {
//     uint16_t rxdata;
//     HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&txdata, (uint8_t *)&rxdata, 1, 500);
//     return rxdata; /* 返回收到的数据 */
// }
