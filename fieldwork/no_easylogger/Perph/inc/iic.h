#ifndef __IIC_H
#define __IIC_H

#include "sys.h"

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

#define E2PROM_IIC_CLK_ENABLE()       \
    do {                              \
        __HAL_RCC_I2C1_CLK_DISABLE(); \
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
    I2C_ERROR           = 1, /* HAL_ERROR */
    I2C_BUSY            = 2, /* HAL BUSY */
    I2C_TIMEOUT         = 3, /* HAL R/W TIME OUT */
    I2C_ERROR_RESOURCE  = 4, /* I2C RESOURCE WRONG */
    I2C_ERROR_PARAMETER = 5, /* INPUT ARG WRONG */
    I2C_RESERVED_1      = 6,
    I2C_RESERVED_2      = 7
} iic_status_t;

iic_status_t iic_init(I2C_HandleTypeDef *hi2c,
                      I2C_TypeDef *instance,
                      uint32_t clock_speed_hz);

iic_status_t iic_deinit(I2C_HandleTypeDef *hi2c);

iic_status_t iic_is_ready(I2C_HandleTypeDef *hi2c,
                          uint16_t dev_addr_7bit,
                          uint32_t trials,
                          uint32_t timeout_ms);

iic_status_t iic_mem_write(I2C_HandleTypeDef *h2ci,
                           uint16_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint16_t mem_addr_size,
                           const uint8_t *buf,
                           uint16_t len,
                           uint32_t timeout_ms);

iic_status_t iic_mem_read(I2C_HandleTypeDef *h2ci,
                          uint16_t dev_addr_7bit,
                          uint16_t mem_addr,
                          uint16_t mem_addr_size,
                          uint8_t *buf,
                          uint16_t len,
                          uint32_t timeout_ms);

typedef struct
{
    void *hi2c;
    iic_status_t (*pf_iic_init)(void *hi2c,
                                void *instance,
                                uint32_t clock_speed_hz);

    iic_status_t (*pf_iic_deinit)(void *hi2c);

    iic_status_t (*pf_iic_is_ready)(void *hi2c,
                                    uint16_t dev_addr_7bit,
                                    uint32_t trials,
                                    uint32_t timeout_ms);

    iic_status_t (*pf_iic_mem_write)(void *h2ic,
                                     uint16_t dev_addr_7bit,
                                     uint16_t mem_addr,
                                     uint16_t mem_addr_size,
                                     const uint8_t *buf,
                                     uint16_t len,
                                     uint32_t timeout_ms);

    iic_status_t (*pf_iic_mem_read)(void *h2ic,
                                    uint16_t dev_addr_7bit,
                                    uint16_t mem_addr,
                                    uint16_t mem_addr_size,
                                    uint8_t *buf,
                                    uint16_t len,
                                    uint32_t timeout_ms);
} iic_driver_t;

#endif