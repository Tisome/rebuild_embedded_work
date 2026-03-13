/**
 * @file RCT6_SPI.c
 * @author Ye Dingzai (yedingzai@126.com)
 * @brief
 * @version 0.1
 * @date 2026-02-12
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "gd32f30x.h"
#include "RCT6_SPI.h"

void SPI0_gpio_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    // PA5(SCK) PA7(MOSI) -> 复用推挽输出
    gpio_init(SPI0_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,
              SPI0_SCK_GPIO_PIN | SPI0_MOSI_GPIO_PIN);
    // PA6(MISO) -> 浮空输入
    gpio_init(SPI0_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,
              SPI0_MISO_GPIO_PIN);
    // PA4(CS) -> 普通GPIO输出（软件控制）
    gpio_init(SPI0_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,
              SPI0_NSS_GPIO_PIN);

    // 默认拉高CS
    SPI0_CS_DISABLE();
}

void SPI0_Init(void)
{
    spi_parameter_struct spi_init_struct;
    SPI0_gpio_Init();

    rcu_periph_clock_enable(RCU_SPI0);

    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE; // Mode0
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_32;
    spi_init_struct.endian = SPI_ENDIAN_MSB;

    spi_init(SPI0, &spi_init_struct);

    spi_enable(SPI0);
}
