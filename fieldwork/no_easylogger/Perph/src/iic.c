#include "iic.h"
#include "elog.h"

#include <stddef.h>

I2C_HandleTypeDef hi2c1 = {0};

static iic_status_t iic_status_from_hal_status(HAL_StatusTypeDef hal_ret)
{
    switch (hal_ret)
    {
    case (HAL_OK): {
        return I2C_OK;
        break;
    }
    case (HAL_BUSY): {
        log_e("i2c_hal_busy");
        return I2C_BUSY;
        break;
    }
    case (HAL_TIMEOUT): {
        log_e("i2c_hal_timeout");
        return I2C_TIMEOUT;
        break;
    }
    case (HAL_ERROR): {
        log_e("i2c_hal_error");
        return I2C_ERROR;
    }
    default: {
        log_e("hal_return something wrong");
        return I2C_ERROR;
        break;
    }
    }
}

static inline uint16_t prv_addr7_to_addr8(uint16_t addr_7bit)
{
    return (uint16_t)(addr_7bit << 1);
}

iic_status_t iic_init(I2C_HandleTypeDef *hi2c,
                      I2C_TypeDef *instance,
                      uint32_t clock_speed_hz)
{

    if (hi2c == NULL || instance == NULL)
    {
        log_e("iic_init: invalid parameter");
        return I2C_ERROR_RESOURCE;
    }

    if (clock_speed_hz == 0U)
    {
        clock_speed_hz = 100000U;
    }

    hi2c->Instance = instance;
    hi2c->Init.ClockSpeed = clock_speed_hz;
    hi2c->Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c->Init.OwnAddress1 = 0U;
    hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.OwnAddress2 = 0U;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    HAL_StatusTypeDef ret = HAL_I2C_Init(hi2c);

    iic_status_t iic_ret = iic_status_from_hal_status(ret);

    if (iic_ret != I2C_OK)
    {
        log_e("HAL_I2C_Init ERROR!");
    }

    return iic_ret;
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hi2c->Instance == E2PROM_IIC_INSTANCE)
    {
        E2PROM_IIC_CLK_ENABLE();
        E2PROM_IIC_SCL_GPIO_CLK_ENABLE();
        E2PROM_IIC_SDA_GPIO_CLK_ENABLE();

        GPIO_InitStruct.Pin = E2PROM_IIC_SCL_GPIO_PIN | E2PROM_IIC_SDA_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(E2PROM_IIC_SCL_GPIO_PORT, &GPIO_InitStruct);
    }
}

iic_status_t iic_deinit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        log_e("i2c resources doesn't exist");
        return I2C_ERROR_RESOURCE;
    }

    HAL_StatusTypeDef ret = HAL_I2C_DeInit(hi2c);

    iic_status_t iic_ret = iic_status_from_hal_status(ret);

    if (iic_ret != I2C_OK)
    {
        log_e("iic deinit failed");
    }

    return iic_ret;
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == E2PROM_IIC_INSTANCE)
    {
        E2PROM_IIC_CLK_ENABLE();
        HAL_GPIO_DeInit(E2PROM_IIC_SCL_GPIO_PORT, E2PROM_IIC_SCL_GPIO_PIN);
        HAL_GPIO_DeInit(E2PROM_IIC_SDA_GPIO_PORT, E2PROM_IIC_SDA_GPIO_PIN);
    }
}

iic_status_t iic_is_ready(I2C_HandleTypeDef *hi2c,
                          uint16_t dev_addr_7bit,
                          uint32_t trials,
                          uint32_t timeout_ms)
{

    if (hi2c == NULL)
    {
        log_e("i2c resources doesn't exist");
        return I2C_ERROR_RESOURCE;
    }

    if (timeout_ms == 0U)
    {
        timeout_ms = IIC_DEFAULT_TIMEOUT_MS;
    }
    HAL_StatusTypeDef ret = HAL_I2C_IsDeviceReady(hi2c,
                                                  prv_addr7_to_addr8(dev_addr_7bit),
                                                  trials,
                                                  timeout_ms);
    iic_status_t iic_ret = iic_status_from_hal_status(ret);
    if (iic_ret != I2C_OK)
    {
        log_e("iic is not ready");
    }
    return iic_ret;
}

iic_status_t iic_mem_write(I2C_HandleTypeDef *hi2c,
                           uint16_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint16_t mem_addr_size,
                           const uint8_t *buf,
                           uint16_t len,
                           uint32_t timeout_ms)
{
    if (hi2c == NULL)
    {
        log_e("i2c resources doesn't exist");
        return I2C_ERROR_RESOURCE;
    }

    if (buf == NULL || len == 0U)
    {
        log_e("i2c write parameter error");
        return I2C_ERROR_PARAMETER;
    }

    if (timeout_ms == 0U)
    {
        timeout_ms = IIC_DEFAULT_TIMEOUT_MS;
    }

    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(hi2c,
                                              prv_addr7_to_addr8(dev_addr_7bit),
                                              mem_addr,
                                              mem_addr_size,
                                              (uint8_t *)buf,
                                              len,
                                              timeout_ms);
    iic_status_t iic_ret = iic_status_from_hal_status(ret);
    if (iic_ret != I2C_OK)
    {
        log_e("iic mem write error");
    }
    return iic_ret;
}

iic_status_t iic_mem_read(I2C_HandleTypeDef *hi2c,
                          uint16_t dev_addr_7bit,
                          uint16_t mem_addr,
                          uint16_t mem_addr_size,
                          uint8_t *buf,
                          uint16_t len,
                          uint32_t timeout_ms)
{
    if (hi2c == NULL)
    {
        log_e("i2c resources doesn't exist");
        return I2C_ERROR_RESOURCE;
    }

    if (buf == NULL || len == 0U)
    {
        log_e("i2c read parameter error");
        return I2C_ERROR_PARAMETER;
    }

    if (timeout_ms == 0U)
    {
        timeout_ms = IIC_DEFAULT_TIMEOUT_MS;
    }

    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(hi2c,
                                             prv_addr7_to_addr8(dev_addr_7bit),
                                             mem_addr,
                                             mem_addr_size,
                                             buf,
                                             len,
                                             timeout_ms);
    iic_status_t iic_ret = iic_status_from_hal_status(ret);
    if (iic_ret != I2C_OK)
    {
        log_e("iic mem read error");
    }
    return iic_ret;
}

static iic_driver_t g_iic_driver;

void iic_driver_init(iic_driver_t *driver)
{
    driver->pf_iic_init = iic_init;
    driver->pf_iic_deinit = iic_deinit;
    driver->pf_iic_is_ready = iic_is_ready;
    driver->pf_iic_mem_write = iic_mem_write;
    driver->pf_iic_mem_read = iic_mem_read;
}