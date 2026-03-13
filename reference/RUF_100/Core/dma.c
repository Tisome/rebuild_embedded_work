#include "dma.h"




void dma_config(void)
{
//    dma_parameter_struct dma_init_struct;

//    /* SPI2 transmit dma config:DMA1,DMA_CH1  */
//    dma_deinit(DMA1, DMA_CH1);
//    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
//    dma_init_struct.memory_addr  = (uint32_t)spi2_send_array;
//    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
//    dma_init_struct.priority     = DMA_PRIORITY_MEDIUM;
//    dma_init(DMA1, DMA_CH1, &dma_init_struct);
//    /* configure DMA mode */
//    dma_circulation_disable(DMA1, DMA_CH1);
//    dma_memory_to_memory_disable(DMA1, DMA_CH1);

//    /* SPI2 receive dma config:DMA1,DMA_CH0  */
//    dma_deinit(DMA1, DMA_CH0);
//    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
//    dma_init_struct.memory_addr  = (uint32_t)spi2_receive_array;
//    dma_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;
//    dma_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;

//    dma_init(DMA1, DMA_CH0, &dma_init_struct);
//    /* configure DMA mode */
//    dma_circulation_disable(DMA1, DMA_CH0);
//    dma_memory_to_memory_disable(DMA1, DMA_CH0);
}
