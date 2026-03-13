#ifndef __I2C_H__
#define __I2C_H__

#include "main.h"
// 返回状态
typedef enum {
    I2C_OK        = 0,  // 操作成功
    I2C_ERR_ADDR  = 1,  // 从设备地址无应答
    I2C_ERR_DATA  = 2,  // 数据传输错误
    I2C_TIMEOUT   = 3,   // 超时
    I2C_BUSY       = 4   // I2C总线忙
} I2C_Status;

// 寄存器地址长度
#define I2C_MEM_ADDR_8BIT  0  // 8位寄存器地址
#define I2C_MEM_ADDR_16BIT 1  // 16位寄存器地址


void e2prom_i2c_config(void);

I2C_Status I2C_WaitFlag(uint32_t i2c_periph, uint32_t flag, 
                       FlagStatus state, uint32_t timeout); 

I2C_Status i2c_send_data(void *hi2c, 
                    uint16_t dst_address, 
                    uint16_t mem_addr, 
                    uint16_t mem_size, 
                    uint8_t  *p_data, 
                    uint16_t size, 
                    uint32_t timeout);

I2C_Status i2c_receive_data(void *hi2c, 
                    uint16_t dst_address, 
                    uint16_t mem_addr, 
                    uint16_t mem_size, 
                    uint8_t  *p_data, 
                    uint16_t size, 
                    uint32_t timeout);

#endif