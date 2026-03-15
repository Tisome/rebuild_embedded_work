#ifndef __IIC_H
#define __IIC_H

#include "sys.h"
#include "stm32f1xx_hal_i2c.h"

/* I2C1 引脚定义（默认 PB6=SCL, PB7=SDA） */
#define IIC1_SCL_GPIO_PORT GPIOB
#define IIC1_SCL_GPIO_PIN  GPIO_PIN_6
#define IIC1_SCL_GPIO_CLK_ENABLE()    \
    do {                              \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0)

#define IIC1_SDA_GPIO_PORT GPIOB
#define IIC1_SDA_GPIO_PIN  GPIO_PIN_7
#define IIC1_SDA_GPIO_CLK_ENABLE()    \
    do {                              \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0)

#define IIC_MEM_ADDR_8BIT  I2C_MEMADD_SIZE_8BIT
#define IIC_MEM_ADDR_16BIT I2C_MEMADD_SIZE_16BIT

#ifndef IIC_DEFAULT_TIMEOUT_MS
#define IIC_DEFAULT_TIMEOUT_MS 100U
#endif

/* E2PROM 引脚定义 */
#if USE_IIC_FOR_E2PROM == 1
#define E2PROM_IIC_HANDLER  hi2c1

#define E2PROM_IIC_INSTANCE I2C1
#define E2PROM_IIC_CLK_ENABLE()      \
    do {                             \
        __HAL_RCC_I2C1_CLK_ENABLE(); \
    } while (0)

#define E2PROM_IIC_SCL_GPIO_PORT       IIC1_SCL_GPIO_PORT
#define E2PROM_IIC_SCL_GPIO_PIN        IIC1_SCL_GPIO_PIN
#define E2PROM_IIC_SCL_GPIO_CLK_ENABLE IIC1_SCL_GPIO_CLK_ENABLE

#define E2PROM_IIC_SDA_GPIO_PORT       IIC1_SDA_GPIO_PORT
#define E2PROM_IIC_SDA_GPIO_PIN        IIC1_SDA_GPIO_PIN
#define E2PROM_IIC_SDA_GPIO_CLK_ENABLE IIC1_SDA_GPIO_CLK_ENABLE

#endif

extern I2C_HandleTypeDef hi2c1;

typedef enum {
    I2C_OK              = 0, /* R/W successfully */
    I2C_ERROR           = 1, /* RUNTIME ERROR WITHOUR CASE MATCHED */
    I2C_ERROR_TIMEOUT   = 2, /* R/W TIME OUT */
    I2C_ERROR_RESOURCE  = 3, /* I2C RESOURCE WRONG */
    I2C_ERROR_PARAMETER = 4, /* INPUT ARG WRONG */
    I2C_RESERVED_1      = 5,
    I2C_RESERVED_2      = 6,
    I2C_RESERVED_3      = 7
} iic_status_t;

void iic1_init(uint32_t clock_speed_hz);

iic_status_t iic1_is_ready(uint16_t dev_addr_7bit, uint32_t trials, uint32_t timeout_ms);
iic_status_t iic1_mem_write(I2C_HandleTypeDef *h2ci,
                            uint16_t dev_addr_7bit,
                            uint16_t mem_addr,
                            uint16_t mem_addr_size,
                            const uint8_t *buf,
                            uint16_t len,
                            uint32_t timeout_ms);
iic_status_t iic1_mem_read(I2C_HandleTypeDef *h2ci,
                           uint16_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint16_t mem_addr_size,
                           uint8_t *buf,
                           uint16_t len,
                           uint32_t timeout_ms);

#endif