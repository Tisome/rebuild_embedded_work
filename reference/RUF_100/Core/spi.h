#ifndef __SPI_H__
#define __SPI_H__

#include "gd32f30x.h"

#define SPI_TIMEOUT_MAX  ((uint32_t)0xFFFFFFFFU)

// 返回状态
typedef enum {
    SPI_OK        = 0,  // 操作成功
    SPI_ERR_ADDR  = 1,  // 从设备地址无应答
    SPI_ERR_DATA  = 2,  // 数据传输错误
    SPI_TIMEOUT   = 3   // 超时
} SPI_Status;

void spi_config(void);
SPI_Status spi_send_data_botransmit(uint32_t spi, const uint8_t *data, uint32_t len); 
SPI_Status spi_send_data_fullduplex(uint32_t spi, const uint8_t *data, uint32_t len); 
SPI_Status spi_receive_data(uint32_t  spi, uint8_t *data, uint32_t len);
#endif
