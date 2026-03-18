#ifndef __SPI_H
#define __SPI_H

#include "sys.h"
// #include "stm32f1xx_hal.h"

/********************************************************************/
/* SPI1 NSS引脚定义 */
#define SPI1_NSS_GPIO_PORT GPIOA
#define SPI1_NSS_GPIO_PIN  GPIO_PIN_4
#define SPI1_NSS_GPIO_CLK_ENABLE()    \
    do {                              \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
    } while (0) /* PA口时钟使能 */

/* SPI1 SCK引脚定义*/
#define SPI1_SCK_GPIO_PORT GPIOA
#define SPI1_SCK_GPIO_PIN  GPIO_PIN_5
#define SPI1_SCK_GPIO_CLK_ENABLE()    \
    do {                              \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
    } while (0) /* PA口时钟使能 */

/* SPI1 MISO引脚定义*/
#define SPI1_MISO_GPIO_PORT GPIOA
#define SPI1_MISO_GPIO_PIN  GPIO_PIN_6
#define SPI1_MISO_GPIO_CLK_ENABLE()   \
    do {                              \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
    } while (0) /* PA口时钟使能 */

/* SPI1 MOSI引脚定义*/
#define SPI1_MOSI_GPIO_PORT GPIOA
#define SPI1_MOSI_GPIO_PIN  GPIO_PIN_7
#define SPI1_MOSI_GPIO_CLK_ENABLE()   \
    do {                              \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
    } while (0) /* PA口时钟使能 */

#define SPI1_SPI SPI1
#define SPI1_SPI_CLK_ENABLE()        \
    do {                             \
        __HAL_RCC_SPI1_CLK_ENABLE(); \
    } while (0) /* SPI1时钟使能 */
#define SPI1_NSS_CS(s)                                                                \
    do {                                                                              \
        if (s)                                                                        \
            HAL_GPIO_WritePin(SPI1_NSS_GPIO_PORT, SPI1_NSS_GPIO_PIN, GPIO_PIN_SET);   \
        else                                                                          \
            HAL_GPIO_WritePin(SPI1_NSS_GPIO_PORT, SPI1_NSS_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0)

/* SPI总线速度设置 */
#define SPI_SPEED_2     0
#define SPI_SPEED_4     1
#define SPI_SPEED_8     2
#define SPI_SPEED_16    3
#define SPI_SPEED_32    4
#define SPI_SPEED_64    5
#define SPI_SPEED_128   6
#define SPI_SPEED_256   7

#define FPGA_CS_ENABLE  0
#define FPGA_CS_DISABLE 1

extern SPI_HandleTypeDef hspi1;

void spi1_init(void);
void spi1_set_speed(uint8_t speed);
// uint16_t xSPI1_read_word(void);
// void spi1_write_word(uint16_t txdata);
// uint16_t xSPI1_read_write_halfword(uint16_t txdata);

#endif /* __SPI_H */
