#include "usart.h"
#include "circular_buffer.h"
#include "stm32f1xx_hal_dma.h"
#include "sys.h"

UART_HandleTypeDef huart1 = {0};
DMA_HandleTypeDef hdma_usart1_rx = {0};

/**
 * @brief  USART1 初始化（DMA + IDLE）
 * @param  baudrate 波特率
 * @retval 无
 */
void uart1_dma_init(uint8_t *uart_rx_buffer, uint32_t databuf_size)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    /* 这里会自动调用 HAL_UART_MspInit() */
    HAL_UART_Init(&huart1);

    /* 启动DMA循环接收 */
    HAL_UART_Receive_DMA(&huart1, uart_rx_buffer, databuf_size);

    /* 开启空闲中断 */
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    /* 开启DMA半传输/全传输中断 */
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_TC);
}

/**
 * @brief  UART底层 MSP 初始化
 * @param  huart UART句柄
 * @retval 无
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        /* 1. 时钟使能 */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        /* 2. GPIO配置 */

        /* TX -> PA9 */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* RX -> PA10 */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* 3. DMA配置：USART1_RX -> DMA1_Channel5 */
        hdma_usart1_rx.Instance = DMA1_Channel5;
        hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
        hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
        HAL_DMA_Init(&hdma_usart1_rx);

        /* 4. 关联UART和DMA */
        __HAL_LINKDMA(huart, hdmarx, hdma_usart1_rx);

        /* 5. 中断配置 */
        HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);

        HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    }
}

/**
 * @brief  USART1发送函数
 * @param  buf 数据地址
 * @param  len 数据长度
 * @retval 无
 */
void usart1_send_bytes(uint8_t *buf, uint16_t len)
{
    HAL_UART_Transmit(&huart1, buf, len, 100);
}

/**
 * @brief  DMA接收半满回调
 * @param  huart UART句柄
 * @retval 无
 */

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 只调整head位置，数据处理放在IDLE中断里做
        uart1_dma_half_irq_callback();
    }
}

/**
 * @brief  DMA接收全满回调
 * @param  huart UART句柄
 * @retval 无
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 只调整head位置，数据处理放在IDLE中断里做
        uart1_dma_full_irq_callback();
    }
}