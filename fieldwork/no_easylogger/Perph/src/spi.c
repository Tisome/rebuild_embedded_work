#include "spi.h"
#include "sys.h"

#include "FreeRTOS.h"
#include "freertos_resources.h"
#include "semphr.h"

SPI_HandleTypeDef hspi1 = {0};
DMA_HandleTypeDef hdma_spi1_rx = {0};

/**
 * @brief       SPI1设备初始化
 * @note        主机模式，8位数据，软件管理NSS，CPOL=0，CPHA=0
 * @param       无
 * @retval      无
 */
void spi1_dma_init(void)
{
    /* 1. 先使能DMA时钟，并初始化SPI1_RX对应的DMA通道 */
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

    /* 2. 使能SPI1时钟 */
    SPI1_SPI_CLK_ENABLE();

    /* 3. 配置SPI1参数 */
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;

    /* 4. 关联SPI1和DMA RX句柄 */
    __HAL_LINKDMA(&hspi1, hdmarx, hdma_spi1_rx);

    /* 5. 初始化SPI1 */
    HAL_SPI_Init(&hspi1);

    /* 6. 使能SPI外设 */
    __HAL_SPI_ENABLE(&hspi1);

    /* 7. 默认拉高片选，禁用从设备 */
    SPI1_NSS_CS(FPGA_CS_DISABLE);
}

/**
 * @brief       SPI底层硬件管脚初始化函数
 * @param       hspi SPI句柄
 * @retval      无
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hspi->Instance == SPI1)
    {
        /* SPI1 GPIO时钟使能 */
        SPI1_SCK_GPIO_CLK_ENABLE();
        SPI1_MISO_GPIO_CLK_ENABLE();
        SPI1_MOSI_GPIO_CLK_ENABLE();
        SPI1_NSS_GPIO_CLK_ENABLE();

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

        /* 如果你还要用DMA中断，这里顺手配NVIC */
        HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    }
}

/**
 * @brief       设置SPI1波特率
 * @param       speed 波特率预分频值
 * @retval      无
 */
void spi1_set_speed(uint8_t speed)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(speed));
    __HAL_SPI_DISABLE(&hspi1);
    hspi1.Instance->CR1 &= 0XFFC7;
    hspi1.Instance->CR1 |= speed << 3;
    __HAL_SPI_ENABLE(&hspi1);
}

/**
 * @brief       SPI接收完成回调函数
 * @param       hspi SPI句柄
 * @retval      无
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xSem_SPI_Rx_Done, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}