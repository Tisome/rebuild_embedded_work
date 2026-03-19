#include "usart.h"

#define USART0_DATA_ADDRESS ((uint32_t)&USART_DATA(USART0))

void usart0_dma_init(uint8_t *rxbuffer, uint32_t buflen)
{
    // usart0 init
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 9600);                     // 波特率
    usart_parity_config(USART0, USART_PM_NONE);           // 无奇偶校验
    usart_word_length_set(USART0, USART_WL_8BIT);         // bit长度
    usart_stop_bit_set(USART0, USART_STB_1BIT);           // 停止位
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE); // 使能发送
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);   // 使能接收
    usart_enable(USART0);

    /* enable USART idle interrupt */
    usart_interrupt_enable(USART0, USART_INT_IDLE);
    nvic_irq_enable(USART0_IRQn, 5, 0);

    // dma init
    dma_parameter_struct dma_init_struct;

    dma_deinit(DMA0, DMA_CH4);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rxbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = buflen;
    dma_init_struct.periph_addr = USART0_DATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);

    dma_circulation_enable(DMA0, DMA_CH4);
    dma_memory_to_memory_disable(DMA0, DMA_CH4);
    /* enable USART DMA for reception */
    usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
    // dma中断设置
    nvic_irq_enable(DMA0_Channel4_IRQn, 4, 0);
    /* enable DMA0 channel4 transfer complete interrupt */
    dma_interrupt_enable(DMA0, DMA_CH4, DMA_INT_FTF);
    dma_interrupt_enable(DMA0, DMA_CH4, DMA_INT_HTF);
    /* enable DMA0 channel4 */
    dma_channel_enable(DMA0, DMA_CH4);
}
/**
 *@brief: usart0发送一个字节
 *@param ch：待发送字节
 *@retval NONE
 */
static void usart0_send_byte(uint8_t ch)
{
    usart_data_transmit(USART0, ch);

    while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
        ;
}

void usart0_send_bytes(uint8_t *ch, uint32_t len)
{
    uint32_t k = 0;
    while (k < len)
    {
        usart0_send_byte(*(ch + k));
        k++;
    }
}