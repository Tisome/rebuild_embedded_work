#include "app_config.h"

#if (RCT6 || CGT6)

#include "gd32f30x.h"
#include "dma.h"
#include "algorithmTask.h"
#include "freertos_resources.h"

#define SPI0_RX_DMA DMA0
#define SPI0_RX_CH DMA_CH1
#define SPI0_TX_DMA DMA0
#define SPI0_TX_CH DMA_CH2

#define SPI0_RX_DMA_IRQn DMA0_Channel1_IRQn


static uint8_t spi0_dummy_tx = 0xFF;
static uint8_t spi0_dummy_rx; // 占位

void DMA0_Init(void)
{
    dma_parameter_struct dma_init_struct;

    rcu_periph_clock_enable(RCU_DMA0);

    // ---------------------- DMA - SPI RX 配置---------------------
    dma_deinit(SPI0_RX_DMA, SPI0_RX_CH);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI0);
    dma_init_struct.memory_addr = (uint32_t)&spi0_dummy_rx; // 占位 后续会改
    dma_init_struct.number = 1;                             // 占位 后续会改
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_init(SPI0_RX_DMA, SPI0_RX_CH, &dma_init_struct);

    // RX完成中断 FTF
    dma_interrupt_enable(SPI0_RX_DMA, SPI0_RX_CH, DMA_INT_FTF);

    // ---------------------- DMA - SPI TX ---------------------
    dma_deinit(SPI0_TX_DMA, SPI0_TX_CH);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI0);
    dma_init_struct.memory_addr = (uint32_t)&spi0_dummy_tx; // 固定 dummy
    dma_init_struct.number = 1;                             // 占位，后续会改
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_DISABLE; // 不递增：重复发 0xFF
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_init(SPI0_TX_DMA, SPI0_TX_CH, &dma_init_struct);
}

// len限制为uint16_t,防止传入过大，进去之后再转类型就可以了
void GD32_SPI0_Receive_DMA(uint8_t *rx_buf, uint16_t len)
{
    // 先停通道
    dma_channel_disable(SPI0_RX_DMA, SPI0_RX_CH);
    dma_channel_disable(SPI0_TX_DMA, SPI0_TX_CH);

    // 清通道flag
    dma_flag_clear(SPI0_RX_DMA, SPI0_RX_CH, DMA_FLAG_G);
    dma_flag_clear(SPI0_TX_DMA, SPI0_TX_CH, DMA_FLAG_G);

    // 写入本次接收的buffer地址和长度
    dma_memory_address_config(SPI0_RX_DMA, SPI0_RX_CH, (uint32_t)rx_buf);
    dma_transfer_number_config(SPI0_RX_DMA, SPI0_RX_CH, (uint32_t)len);

    // Tx dummy 也要发len个字节
    dma_transfer_number_config(SPI0_TX_DMA, SPI0_TX_CH, len);

    // 打开SPI DMA 请求
    spi_dma_enable(SPI0, SPI_DMA_RECEIVE);
    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);

    // 6) 先开 RX 再开 TX
    dma_channel_enable(SPI0_RX_DMA, SPI0_RX_CH);
    dma_channel_enable(SPI0_TX_DMA, SPI0_TX_CH);
}

void DMA0_Channel1_IRQHandler(void)
{
    if (dma_interrupt_flag_get(SPI0_RX_DMA, SPI0_RX_CH, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(SPI0_RX_DMA, SPI0_RX_CH, DMA_INT_FLAG_FTF);
        // 关DMA请求 停通道
        dma_channel_disable(SPI0_RX_DMA, SPI0_RX_CH);
        dma_channel_disable(SPI0_TX_DMA, SPI0_TX_CH);
        spi_dma_disable(SPI0, SPI_DMA_RECEIVE);
        spi_dma_disable(SPI0, SPI_DMA_TRANSMIT);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        /* 通知SPI_Rx任务当前数据已经接收完成 */
        xSemaphoreGiveFromISR(xSem_SPI_Rx_Done, &xHigherPriorityTaskWoken);
        /* 强制任务切换 */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

#endif


