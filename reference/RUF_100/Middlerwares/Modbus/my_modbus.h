#ifndef __MY_MODBUS_H__
#define __MY_MODBUS_H__

#include <stdint.h>
#include "data.h"

// Modbus 协议定义
extern Parameters_t g_parameters;
#define MODBUS_ADDRESS (g_parameters.modbus_addr)       // 本设备地址
#define MODBUS_CRC_LENGTH 2        // CRC校验长度

// Modbus 功能码
#define MODBUS_READ_COILS 0x01
#define MODBUS_READ_DISCRETE_INPUTS 0x02
#define MODBUS_READ_HOLDING_REGISTERS    0x03
#define MODBUS_READ_INPUT_REGISTERS 0x04
#define MODBUS_WRITE_SINGLE_COIL 0x05
#define MODBUS_WRITE_SINGLE_REGISTER 0x06
#define MODBUS_WRITE_MULTIPLE_REGISTERS 0x10

// Modbus 异常码
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION 0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS 0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE 0x03
#define MODBUS_EXCEPTION_SERVER_FAILURE 0x04

//Modbus 超时时间
#define MODBUS_TIMEOUT_MS    (50)
// Modbus 帧状态
typedef enum {
    MODBUS_STATE_IDLE,              // 空闲状态，等待新帧
    MODBUS_STATE_ADDRESS,           // 已接收地址字节
    MODBUS_STATE_FUNCTION,          // 已接收功能码
    MODBUS_STATE_DATA,              // 接收数据字节
    MODBUS_STATE_CRC_LOW,           // 接收CRC低字节
    MODBUS_STATE_CRC_HIGH,          // 接收CRC高字节
} modbus_recv_state_t;

// Modbus 帧结构
typedef struct {
    modbus_recv_state_t state;      // 当前状态
    uint8_t address;                // 设备地址
    uint8_t function;               // 功能码
    uint8_t data[128];              // 数据缓冲区
    uint16_t data_length;           // 当前数据长度
    uint16_t expected_length;       // 期望数据长度
    uint16_t crc;                   // 接收的CRC值
    uint16_t calculated_crc;        // 计算的CRC值
    uint32_t last_char_time;        // 最后字节接收时间
} modbus_parser_t;
// 声明解析函数
void process_modbus_frame(modbus_parser_t *parser);
// 更高效的逐字节CRC计算
uint16_t crc_update(uint16_t crc, uint8_t data);
#endif
