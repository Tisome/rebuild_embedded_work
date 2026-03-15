#include "at24cxx.h"

#include <stddef.h>

static HAL_StatusTypeDef at24cxx_check_args(const at24cxx_dev_t *dev,
                                            uint16_t mem_addr,
                                            uint16_t len)
{
    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    if ((dev->mem_addr_size != IIC_MEM_ADDR_8BIT) &&
        (dev->mem_addr_size != IIC_MEM_ADDR_16BIT))
    {
        return HAL_ERROR;
    }

    if (dev->page_size == 0U)
    {
        return HAL_ERROR;
    }

    if (len == 0U)
    {
        return HAL_ERROR;
    }

    if (dev->mem_size_bytes > 0U)
    {
        uint32_t end_addr = (uint32_t)mem_addr + (uint32_t)len;
        if (end_addr > dev->mem_size_bytes)
        {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

void at24cxx_make_default(at24cxx_dev_t *dev)
{
    if (dev == NULL)
    {
        return;
    }

    dev->i2c_addr_7bit = AT24CXX_DEFAULT_I2C_ADDR_7BIT;
    dev->page_size = AT24CXX_DEFAULT_PAGE_SIZE;
    dev->mem_addr_size = AT24CXX_DEFAULT_MEM_ADDR_SIZE;
    dev->mem_size_bytes = 0U;
    dev->write_cycle_timeout_ms = AT24CXX_DEFAULT_WRITE_CYCLE_TIMEOUTMS;
}

HAL_StatusTypeDef at24cxx_is_ready(const at24cxx_dev_t *dev,
                                   uint32_t trials,
                                   uint32_t timeout_ms)
{
    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    if (trials == 0U)
    {
        trials = 1U;
    }

    return iic1_is_ready(dev->i2c_addr_7bit, trials, timeout_ms);
}

HAL_StatusTypeDef at24cxx_read(const at24cxx_dev_t *dev,
                               uint16_t mem_addr,
                               uint8_t *buf,
                               uint16_t len,
                               uint32_t timeout_ms)
{
    if (buf == NULL)
    {
        return HAL_ERROR;
    }

    if (at24cxx_check_args(dev, mem_addr, len) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return iic1_mem_read(dev->i2c_addr_7bit,
                         mem_addr,
                         dev->mem_addr_size,
                         buf,
                         len,
                         timeout_ms);
}

HAL_StatusTypeDef at24cxx_write(const at24cxx_dev_t *dev,
                                uint16_t mem_addr,
                                const uint8_t *buf,
                                uint16_t len,
                                uint32_t timeout_ms)
{
    if (buf == NULL)
    {
        return HAL_ERROR;
    }

    if (at24cxx_check_args(dev, mem_addr, len) != HAL_OK)
    {
        return HAL_ERROR;
    }

    uint16_t cur_addr = mem_addr; // 当前写的地址
    uint16_t done = 0U;           // 目前已经写了的数据长度

    while (done < len) // 当数据还没有写完的时候
    {
        uint16_t page_offset = (uint16_t)(cur_addr % dev->page_size);  // 本页开始写的时候的页内偏移
        uint16_t page_left = (uint16_t)(dev->page_size - page_offset); // 本页剩下可以写的数据长度
        uint16_t remain = (uint16_t)(len - done);                      // 整个数据包的未写的剩下长度
        uint16_t chunk = (page_left < remain) ? page_left : remain;    // 在本页剩余长度和整个数据包的剩下长度中选一个更小的，即为当前要写的数据长度

        HAL_StatusTypeDef ret = iic1_mem_write(dev->i2c_addr_7bit,
                                               cur_addr,
                                               dev->mem_addr_size,
                                               &buf[done],
                                               chunk,
                                               timeout_ms);
        if (ret != HAL_OK)
        {
            return ret;
        }

        ret = iic1_is_ready(dev->i2c_addr_7bit, 10U, dev->write_cycle_timeout_ms);
        if (ret != HAL_OK)
        {
            return ret;
        }

        cur_addr = (uint16_t)(cur_addr + chunk);
        done = (uint16_t)(done + chunk);
    }

    return HAL_OK;
}