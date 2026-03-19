#ifndef __MODBUS_PROTOCOL_H
#define __MODBUS_PROTOCOL_H

#include "data.h"

extern Pipe_Parameters_t g_parameters;

#define MODBUS_SLAVE_ADDR (g_parameters.modbus_addr)

// Modbus 功能码
#define MODBUS_READ_COILS               0x01 // 读线圈
#define MODBUS_READ_DISCRETE_INPUTS     0x02 // 读离散输入
#define MODBUS_READ_HOLDING_REGISTERS   0x03 // 读保持寄存器
#define MODBUS_READ_INPUT_REGISTERS     0x04 // 读输入寄存器
#define MODBUS_WRITE_SINGLE_COIL        0x05 // 写单个线圈
#define MODBUS_WRITE_SINGLE_REGISTER    0x06 // 写单个寄存器
#define MODBUS_WRITE_MULTIPLE_REGISTERS 0x10 // 写多个寄存器

// Modbus 异常码
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION     0x01 // 非法功能码
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS 0x02 // 非法数据地址
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE   0x03 // 非法数据值
#define MODBUS_EXCEPTION_SERVER_FAILURE       0x04 // 服务器故障

#define MODBUS_TIMEOUT_MS                     (50)

typedef enum {
    MODBUS_STATE_IDLE     = 0, // 空闲状态
    MODBUS_STATE_ADDRESS  = 1, // 已接收地址
    MODBUS_STATE_FUNCTION = 2, // 已接收功能码
    MODBUS_STATE_DATA     = 3, // 已接收数据
    MODBUS_STATE_CRC_LOW  = 4, // 已接收CRC低字节
    MODBUS_STATE_CRC_HIGH = 5  // 已接收CRC高字节
} modbus_state_t;

typedef struct
{
    modbus_state_t state;          // 当前状态
    uint8_t address;               // 接收的地址
    uint8_t function;              // 接收的功能码
    uint8_t data[128];             // 接收的数据，最大长度为128字节
    uint16_t data_length;          // 接收的数据长度
    uint16_t expected_data_length; // 预期的数据长度，根据功能码确定
    uint16_t crc;                  // 接收的CRC校验值
    uint16_t calculated_crc;       // 计算的CRC校验值
    uint32_t last_char_time;       // 上次接收字符的时间戳，用于判断是否超时
} modbus_parser_t;

#endif