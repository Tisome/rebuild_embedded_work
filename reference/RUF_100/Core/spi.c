#include "spi.h"
#include "elog.h"
void spi_config(void)
{
    spi_parameter_struct spi_init_struct;

    /* TFT SPI2 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_2;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);

    spi_enable(SPI2);

    /* GP22 SPI1 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_4;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);

    spi_enable(SPI1);
}


/**
 * SPI发送指定长度的数据
 * 原理：循环发送每个字节，等待发送缓冲区为空后写入数据
 */
SPI_Status spi_send_data_botransmit(uint32_t spi, const uint8_t *data, uint32_t len) 
{
    uint32_t i;
    uint32_t timeout = 0;

    SPI_Status status = SPI_OK;

    for (i = 0; i < len; i++) 
    {
        // 等待发送缓冲区为空（TBE标志置位）
        while (RESET == spi_i2s_flag_get(spi, SPI_FLAG_TBE))
        {
            // 超时处理
            timeout++;
            if (timeout > SPI_TIMEOUT_MAX) 
            {
                log_e("spi_send_data: Timeout waiting for TBE");
                return SPI_TIMEOUT;
            }
        }

        // 发送一个字节
        spi_i2s_data_transmit(spi, data[i]);

        timeout = 0;
    }
    // 等待最后一个字节发送完成（避免提前退出导致数据未发完）
    while ((SET   == spi_i2s_flag_get(spi, SPI_FLAG_TRANS)) &&
           (RESET == spi_i2s_flag_get(spi, SPI_FLAG_TBE))) 
    {
    }
    return status;
}
/**
 * SPI发送指定长度的数据
 * 原理：循环发送每个字节，等待发送缓冲区为空后写入数据
 */
SPI_Status spi_send_data_fullduplex(uint32_t spi, const uint8_t *data, uint32_t len) 
{
    uint32_t i;
    uint32_t timeout = 0;

    SPI_Status status = SPI_OK;

    for (i = 0; i < len; i++) 
    {
        // 等待发送缓冲区为空（TBE标志置位）
        while (RESET == spi_i2s_flag_get(spi, SPI_FLAG_TBE))
        {
            // 超时处理
            timeout++;
            if (timeout > SPI_TIMEOUT_MAX) 
            {
                log_e("spi_send_data: Timeout waiting for TBE");
                return SPI_TIMEOUT;
            }
        }

        // 发送一个字节
        spi_i2s_data_transmit(spi, data[i]);

        while (RESET == spi_i2s_flag_get(spi, SPI_FLAG_RBNE))
        {
            // 超时处理
            timeout++;
            if (timeout > SPI_TIMEOUT_MAX) 
            {
                log_e("spi_receive_data: Timeout waiting for RBNE");
                return SPI_TIMEOUT;
            }
        }
        //从fifo中清空无效数据
        spi_i2s_data_receive(spi);

        timeout = 0;
    }
    // 等待最后一个字节发送完成（避免提前退出导致数据未发完）
    while ((SET   == spi_i2s_flag_get(spi, SPI_FLAG_TRANS)) &&
           (RESET == spi_i2s_flag_get(spi, SPI_FLAG_TBE))) 
    {
    }
    return status;
}

/**
 * SPI接收指定长度的数据
 * 原理：SPI接收需通过发送dummy字节触发时钟，每接收1字节需发送1个dummy
 */
SPI_Status spi_receive_data(uint32_t  spi, uint8_t *data, uint32_t len) 
{
    uint32_t i;
    SPI_Status status = SPI_OK;

    uint32_t timeout = 0;

    for (i = 0; i < len; i++) 
    {
        // 1. 发送一个dummy字节（触发SPI时钟，从机才能发送数据）
        while (RESET == spi_i2s_flag_get(spi, SPI_FLAG_TBE));
        spi_i2s_data_transmit(spi, 0x00);  // 发送0xFF作为dummy

        // 2. 等待接收缓冲区非空（RBNE标志置位）
         while (RESET == spi_i2s_flag_get(spi, SPI_FLAG_RBNE))
         {
                // 超时处理
                timeout++;
                if (timeout > SPI_TIMEOUT_MAX) 
                {
                    log_e("spi_receive_data: Timeout waiting for RBNE");
                    return SPI_TIMEOUT;
                }
         }
        // 读取接收数据
        data[i] = spi_i2s_data_receive(spi);

        timeout = 0;
    }
    return status;
}
