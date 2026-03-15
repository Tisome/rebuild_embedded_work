#include "iic.h"
#include "elog.h"

#include <stddef.h>

I2C_HandleTypeDef E2PROM_IIC_HANDLER = {0};

static inline uint16_t prv_addr7_to_addr8(uint16_t addr_7bit)
{
    return (uint16_t)(addr_7bit << 1);
}

void e2prom_i2c_init(uint32_t clock_speed_hz)
{
    if (clock_speed_hz == 0U)
    {
        clock_speed_hz = 100000U;
    }

    E2PROM_IIC_CLK_ENABLE();

    E2PROM_IIC_HANDLER.Instance = E2PROM_IIC_INSTANCE;
    E2PROM_IIC_HANDLER.Init.ClockSpeed = clock_speed_hz;
    E2PROM_IIC_HANDLER.Init.DutyCycle = I2C_DUTYCYCLE_2;
    E2PROM_IIC_HANDLER.Init.OwnAddress1 = 0U;
    E2PROM_IIC_HANDLER.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    E2PROM_IIC_HANDLER.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    E2PROM_IIC_HANDLER.Init.OwnAddress2 = 0U;
    E2PROM_IIC_HANDLER.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    E2PROM_IIC_HANDLER.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&E2PROM_IIC_HANDLER) != HAL_OK)
    {
        log_e("HAL_I2C_Init ERROR!");
    }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == E2PROM_IIC_INSTANCE)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        E2PROM_IIC_SCL_GPIO_CLK_ENABLE();
        E2PROM_IIC_SDA_GPIO_CLK_ENABLE();

        GPIO_InitStruct.Pin = E2PROM_IIC_SCL_GPIO_PIN | E2PROM_IIC_SDA_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(E2PROM_IIC_SCL_GPIO_PORT, &GPIO_InitStruct);
    }
}

iic_status_t iic_is_ready(I2C_HandleTypeDef *hi2c,
                          uint16_t dev_addr_7bit,
                          uint32_t trials,
                          uint32_t timeout_ms)
{
    if (timeout_ms == 0U)
    {
        timeout_ms = IIC_DEFAULT_TIMEOUT_MS;
    }
    uint8_t ret = HAL_I2C_IsDeviceReady(&hi2c,
                                        prv_addr7_to_addr8(dev_addr_7bit),
                                        trials,
                                        timeout_ms);
    if (ret != HAL_OK)
    {
        log_e("i2c timeout error");
        return I2C_ERROR_TIMEOUT;
    }
    return I2C_OK;
}

iic_status_t iic_mem_write(I2C_HandleTypeDef *hi2c,
                           uint16_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint16_t mem_addr_size,
                           const uint8_t *buf,
                           uint16_t len,
                           uint32_t timeout_ms)
{
    if (buf == NULL || len == 0U)
    {
        log_e("i2c write parameter error");
        return I2C_ERROR_PARAMETER;
    }

    if (timeout_ms == 0U)
    {
        timeout_ms = IIC_DEFAULT_TIMEOUT_MS;
    }

    uint8_t ret = HAL_I2C_Mem_Write(hi2c,
                                    prv_addr7_to_addr8(dev_addr_7bit),
                                    mem_addr,
                                    mem_addr_size,
                                    (uint8_t *)buf,
                                    len,
                                    timeout_ms);
    if (ret != HAL_OK)
    {
        log_e("HAL_I2C_Mem_Write error");
        return I2C_ERROR;
    }
    return I2C_OK;
}

iic_status_t iic1_mem_read(I2C_HandleTypeDef *hi2c,
                           uint16_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint16_t mem_addr_size,
                           uint8_t *buf,
                           uint16_t len,
                           uint32_t timeout_ms)
{
    if (buf == NULL || len == 0U)
    {
        return HAL_ERROR;
    }

    if (timeout_ms == 0U)
    {
        timeout_ms = IIC_DEFAULT_TIMEOUT_MS;
    }

    return HAL_I2C_Mem_Read(hi2c,
                            prv_addr7_to_addr8(dev_addr_7bit),
                            mem_addr,
                            mem_addr_size,
                            buf,
                            len,
                            timeout_ms);
}