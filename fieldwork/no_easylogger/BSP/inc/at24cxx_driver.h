#ifndef __AT24CXX_DRIVER_H
#define __AT24CXX_DRIVER_H

#include "iic.h"

#include <stdint.h>

/* 常见器件默认配置（可按板级硬件修改） */
#if E2PROM_AT24C32
#define AT24CXX_DEFAULT_I2C_ADDR_7BIT         0x50U              // 7bit的at24cxx器件iic地址
#define AT24CXX_DEFAULT_PAGE_SIZE             32U                // 内部数据页大小(B)
#define AT24CXX_DEFAULT_MEM_ADDR_SIZE         IIC_MEM_ADDR_16BIT // 内部数据地址长度
#define AT24CXX_DEFAULT_WRITE_CYCLE_TIMEOUTMS 10U                // 写超时时间

#elif E2PROM_AT24C08
#define AT24CXX_DEFAULT_I2C_ADDR_7BIT         0x50U
#define AT24CXX_DEFAULT_PAGE_SIZE             16U
#define AT24CXX_DEFAULT_MEM_ADDR_SIZE         IIC_MEM_ADDR_8BIT
#define AT24CXX_DEFAULT_WRITE_CYCLE_TIMEOUTMS 10U
#endif

typedef enum {
    AT24CXX_OK              = 0, /* R/W successfully */
    AT24CXX_ERROR           = 1, /* runtime_error and no case matched */
    AT24CXX_ERROR_TIMEOUT   = 2, /* e2prom r/w */
    AT24CXX_ERROR_RESOURCE  = 3, /* e2prom resource  */
    AT24CXX_ERROR_PARAMETER = 4,
    AT24CXX_ERROR_NO_MEMORY = 5,
    AT24CXX_RESERVED_1      = 6,
    AT24CXX_RESERVED_2      = 7

} at24cxx_status_t;

typedef struct
{
    void *hi2c;
    at24cxx_status_t (*pf_iic_init)(void *); // iic init 这是一个函数指针，传入参数是void *
    at24cxx_status_t (*pf_iic_deinit)(void *);
    at24cxx_status_t (*pf_iic_mem_write)(void *h2ic,
                                         uint16_t dev_addr_7bit,
                                         uint16_t mem_addr,
                                         uint16_t mem_addr_size,
                                         const uint8_t *buf,
                                         uint16_t len,
                                         uint32_t timeout_ms);
    at24cxx_status_t (*pf_iic_mem_read)(void *h2ic,
                                        uint16_t dev_addr_7bit,
                                        uint16_t mem_addr,
                                        uint16_t mem_addr_size,
                                        uint8_t *buf,
                                        uint16_t len,
                                        uint32_t timeout_ms);
} iic_driver_t;

/*
 * @brief AT24Cxx 设备描述
 *
 * mem_addr_size: IIC_MEM_ADDR_8BIT / IIC_MEM_ADDR_16BIT
 * mem_size_bytes: 可选边界保护，0 表示不做边界检查
 */
typedef struct
{
    uint16_t i2c_addr_7bit;
    uint16_t page_size;
    uint16_t mem_addr_size;
    uint16_t mem_size_bytes;
    uint32_t write_cycle_timeout_ms;
} at24cxx_dev_t;

void at24cxx_make_default(at24cxx_dev_t *dev);

HAL_StatusTypeDef at24cxx_is_ready(const at24cxx_dev_t *dev,
                                   uint32_t trials,
                                   uint32_t timeout_ms);

HAL_StatusTypeDef at24cxx_read(const at24cxx_dev_t *dev,
                               uint16_t mem_addr,
                               uint8_t *buf,
                               uint16_t len,
                               uint32_t timeout_ms);

HAL_StatusTypeDef at24cxx_write(const at24cxx_dev_t *dev,
                                uint16_t mem_addr,
                                const uint8_t *buf,
                                uint16_t len,
                                uint32_t timeout_ms);

/************************* at24cxx driver struct *************************/

typedef struct
{
    iic_driver_t *p_iic_driver_instance;
    at24cxx_dev_t *p_at24cxx_dev_info;

} at24cxx_driver_t;

#endif