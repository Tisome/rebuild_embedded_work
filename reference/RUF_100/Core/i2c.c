#include "i2c.h"
#include "main.h"

void i2c_bus_reset()
{
    i2c_deinit(EEPROM_I2C);
    /* configure SDA/SCL for GPIO */
    GPIO_BC(EEPROM_I2C_GPIO_PORT) |= EEPROM_I2C_SCL_PIN;
    GPIO_BC(EEPROM_I2C_GPIO_PORT) |= EEPROM_I2C_SDA_PIN;
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, EEPROM_I2C_SCL_PIN);
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, EEPROM_I2C_SDA_PIN);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    GPIO_BOP(EEPROM_I2C_GPIO_PORT) |= EEPROM_I2C_SCL_PIN;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    GPIO_BOP(EEPROM_I2C_GPIO_PORT) |= EEPROM_I2C_SDA_PIN;
    /* connect I2C_SCL_PIN to I2C_SCL */
    /* connect I2C_SDA_PIN to I2C_SDA */
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, EEPROM_I2C_SCL_PIN);
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, EEPROM_I2C_SDA_PIN);
}
void e2prom_i2c_config(void)
{
    i2c_bus_reset();
    /* configure I2C clock */
    i2c_clock_config(EEPROM_I2C, 100000, I2C_DTCY_2);
     /* configure I2C address */
    i2c_mode_addr_config(EEPROM_I2C, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0xFF);
    /* enable I2C0 */
    i2c_enable(EEPROM_I2C);
    /* enable acknowledge */
    i2c_ack_config(EEPROM_I2C, I2C_ACK_ENABLE);
}

I2C_Status I2C_WaitFlag(uint32_t i2c_periph, uint32_t flag, FlagStatus state, uint32_t timeout) 
{
    uint32_t tick_start = get_delay_tick();
    while (i2c_flag_get(i2c_periph, flag) != state) 
    {
        if (get_delay_tick() - tick_start > timeout) 
        {
            return I2C_TIMEOUT;
        }
    }
    return I2C_OK;
}
I2C_Status i2c_send_data(void *i2c_handle, 
                     uint16_t dst_address, 
                        uint16_t mem_addr, 
                        uint16_t mem_size, 
                         uint8_t  *p_data, 
                            uint16_t size, 
                         uint32_t timeout)
{
    uint32_t i2c_periph = (uint32_t)i2c_handle;
    // if (I2C_WaitFlag(i2c_periph, I2C_FLAG_I2CBSY, SET, timeout) != I2C_OK) 
    // {
    //     i2c_stop_on_bus(i2c_periph);  // 失败则发送停止信号
    //     log_e("i2c_send_busy timeout");
    //     return I2C_TIMEOUT;
    // }
    // 1. 发送起始信号
    i2c_start_on_bus(i2c_periph);
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_SBSEND, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);  // 失败则发送停止信号
        log_e("i2c_send_start timeout");
        return I2C_TIMEOUT;
    }

    // 2. 发送从设备写地址
    i2c_master_addressing(i2c_periph, dst_address, I2C_TRANSMITTER);
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_ADDSEND, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_address timeout");
        return (i2c_flag_get(i2c_periph, I2C_FLAG_AERR) ? I2C_ERR_ADDR : I2C_TIMEOUT);
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);  

    // 3. 发送寄存器地址
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_TBE, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_start timeout");
        return I2C_TIMEOUT;
    }
    if (mem_size == I2C_MEM_ADDR_16BIT) 
    {
        // 16位地址：先发送高8位
        i2c_data_transmit(i2c_periph, (mem_addr >> 8) & 0xFF);
        if (I2C_WaitFlag(i2c_periph, I2C_FLAG_BTC, SET, timeout) != I2C_OK) 
        {
            i2c_stop_on_bus(i2c_periph);
            log_e("i2c_send_reg high timeout");
            return I2C_TIMEOUT;
        }
    }
    // 发送低8位
    i2c_data_transmit(i2c_periph, mem_addr & 0xFF);
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_BTC, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_reg low timeout");
        return I2C_TIMEOUT;
    }

    // 4. 发送数据
    for (uint16_t i = 0; i < size; i++) 
    {
        i2c_data_transmit(i2c_periph, p_data[i]);
        if (I2C_WaitFlag(i2c_periph, I2C_FLAG_BTC, SET, timeout) != I2C_OK) 
        {
            log_e("i2c_send_data timeout");
            i2c_stop_on_bus(i2c_periph);
            return I2C_TIMEOUT;
        }
    }

    // 5. 发送停止信号，结束通信
    i2c_stop_on_bus(i2c_periph);
    while(I2C_CTL0(i2c_periph)&I2C_CTL0_STOP);
    return I2C_OK;
}
I2C_Status i2c_receive_data(void *i2c_handle, 
                        uint16_t dst_address, 
                           uint16_t mem_addr, 
                           uint16_t mem_size, 
                            uint8_t  *p_data, 
                               uint16_t size, 
                            uint32_t timeout)
{
    uint32_t i2c_periph = (uint32_t)i2c_handle;
    // if (I2C_WaitFlag(i2c_periph, I2C_FLAG_I2CBSY, SET, timeout) != I2C_OK) 
    // {
    //     i2c_stop_on_bus(i2c_periph);  // 失败则发送停止信号
    //     log_e("i2c_send_busy timeout");
    //     return I2C_TIMEOUT;
    // }
    // 1. 发送起始信号
    i2c_start_on_bus(i2c_periph);
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_SBSEND, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_start timeout");
        return I2C_TIMEOUT;
    }

    // 2. 发送从设备写地址
    i2c_master_addressing(i2c_periph, dst_address, I2C_TRANSMITTER);
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_ADDSEND, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_address timeout");
        return (i2c_flag_get(i2c_periph, I2C_FLAG_AERR) ? I2C_ERR_ADDR : I2C_TIMEOUT);
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    // 3. 发送寄存器地址（8位或16位）
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_TBE, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_start timeout");
        return I2C_TIMEOUT;
    }
    if (mem_size == I2C_MEM_ADDR_16BIT) 
    {
        i2c_data_transmit(i2c_periph, (mem_addr >> 8) & 0xFF);  // 高8位
        if (I2C_WaitFlag(i2c_periph, I2C_FLAG_BTC, SET, timeout) != I2C_OK) 
        {
            i2c_stop_on_bus(i2c_periph);
            log_e("i2c_send_reg high timeout");
            return I2C_TIMEOUT;
        }
    }
    i2c_data_transmit(i2c_periph, mem_addr & 0xFF);  // 低8位
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_BTC, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_reg low timeout");
        return I2C_TIMEOUT;
    }

    // 4. 发送重复起始信号
    // if (I2C_WaitFlag(i2c_periph, I2C_FLAG_I2CBSY, SET, timeout) != I2C_OK) 
    // {
    //     log_e("i2c bus restart busy");
    //     return I2C_BUSY;
    // }
    i2c_start_on_bus(i2c_periph);
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_SBSEND, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_restart timeout");
        return I2C_TIMEOUT;
    }

    // 5. 发送从设备读地址
    i2c_master_addressing(i2c_periph, dst_address, I2C_RECEIVER);
    if (I2C_WaitFlag(i2c_periph, I2C_FLAG_ADDSEND, SET, timeout) != I2C_OK) 
    {
        i2c_stop_on_bus(i2c_periph);
        log_e("i2c_send_address timeout");
        return (i2c_flag_get(i2c_periph, I2C_FLAG_AERR) ? I2C_ERR_ADDR : I2C_TIMEOUT);
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    // 6. 接收数据
    for (uint16_t i = 0; i < size; i++) 
    {
        if (i == size - 1) {
            // 最后一个字节：禁止应答，准备发送停止信号
            i2c_ack_config(i2c_periph, I2C_ACK_DISABLE);
        }
        // 等待接收缓冲区非空
        if (I2C_WaitFlag(i2c_periph, I2C_FLAG_RBNE, SET, timeout) != I2C_OK) 
        {
            i2c_stop_on_bus(i2c_periph);
            i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);  // 恢复应答使能
            log_e("i2c_receive_data timeout");
            return I2C_TIMEOUT;
        }
        p_data[i] = i2c_data_receive(i2c_periph);  // 读取数据
    }

    // 7. 发送停止信号，恢复应答使能
    i2c_stop_on_bus(i2c_periph);
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);  
    while(I2C_CTL0(i2c_periph)&I2C_CTL0_STOP);
    return I2C_OK;
}