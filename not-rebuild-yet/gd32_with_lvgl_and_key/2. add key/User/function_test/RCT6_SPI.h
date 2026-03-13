#ifndef __TEST_RCT6_SPI_H
#define __TEST_RCT6_SPI_H

#include "gd32f30x.h"

#define SPI0_GPIO_PORT GPIOA
#define SPI0_NSS_GPIO_PIN GPIO_PIN_4
#define SPI0_SCK_GPIO_PIN GPIO_PIN_5
#define SPI0_MISO_GPIO_PIN GPIO_PIN_6
#define SPI0_MOSI_GPIO_PIN GPIO_PIN_7

#define SPI0_CS_DISABLE()                                \
    do                                                   \
    {                                                    \
        gpio_bit_set(SPI0_GPIO_PORT, SPI0_NSS_GPIO_PIN); \
    } while (0)

#define SPI0_CS_ENABLE()                                   \
    do                                                     \
    {                                                      \
        gpio_bit_reset(SPI0_GPIO_PORT, SPI0_NSS_GPIO_PIN); \
    } while (0)

void SPI0_Init(void);

#endif
