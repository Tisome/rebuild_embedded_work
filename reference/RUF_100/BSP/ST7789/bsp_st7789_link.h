#ifndef __ST7789_LINK
#define __ST7789_LINK

#include "main.h"
#include "dma.h"
#include "spi.h"
#include "bsp_st7789_driver.h"

static inline uint8_t spi_transmit_data(const uint8_t *pData, uint32_t dataLength)
{
	spi_send_data_botransmit(SPI2, pData, dataLength);
	return 0;
}

static inline uint8_t spi_transmit_data_dma(const uint8_t *pData, uint32_t dataLength)
{
    dma_parameter_struct dma_init_struct;

    dma_deinit(TFT_DMA, TFT_DMA_CH);
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
    dma_init_struct.memory_addr  = (uint32_t)pData;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority     = DMA_PRIORITY_LOW;
    dma_init_struct.number       = dataLength;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(TFT_DMA, TFT_DMA_CH, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(TFT_DMA, TFT_DMA_CH);
    dma_memory_to_memory_disable(TFT_DMA, TFT_DMA_CH);

    dma_channel_enable(TFT_DMA, TFT_DMA_CH);
    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);
    while(!dma_flag_get(TFT_DMA,TFT_DMA_CH, DMA_FLAG_FTF)) {
    }
	
	return 0;
}

static inline uint8_t gpio_write_reset_pin(uint8_t pinState)
{
	if(0 == pinState)
	{
		gpio_bit_reset(TFT_RESET_GPIO_Port, TFT_RESET_Pin);
	}
	else
	{
		gpio_bit_set(TFT_RESET_GPIO_Port, TFT_RESET_Pin);
	}
	return pinState;
}

static inline uint8_t gpio_write_cs_pin(uint8_t pinState)
{
	if(0 == pinState)
	{
		gpio_bit_reset(TFT_CS_GPIO_Port, TFT_CS_Pin);
	}
	else
	{
		gpio_bit_set(TFT_CS_GPIO_Port, TFT_CS_Pin);
	}
	return pinState;
}

static inline uint8_t gpio_write_dc_pin(uint8_t pinState)
{
	if(0 == pinState)
	{
		gpio_bit_reset(TFT_DC_GPIO_Port, TFT_DC_Pin);
	}
	else
	{
        gpio_bit_set(TFT_DC_GPIO_Port, TFT_DC_Pin);
	}
	return pinState;
}







#endif /* __ST7789_LINK */
