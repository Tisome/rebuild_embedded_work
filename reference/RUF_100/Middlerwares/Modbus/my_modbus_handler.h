#ifndef __MY_MODBUS_HANDLER_H__
#define __MY_MODBUS_HANDLER_H__
#include "my_modbus.h"
#include "data.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "elog.h"
// 地址映射定义
#define COIL_START_ADDR       0x0000  // 线圈起始地址
#define DISCRETE_INPUT_START  0x0000  // 离散输入起始地址
#define INPUT_REG_START       0x1000  // 输入寄存器起始地址
#define HOLDING_REG_START     0x0008  // 保持寄存器起始地址

// Modbus 数据存储
#define COIL_SIZE             8       // 8个线圈 (0x0000-0x0007)
#define DISCRETE_INPUT_SIZE   8      // 16个位 (0x1000-0x100F)
#define INPUT_REG_SIZE        1      // 12个寄存器 (0x3000-0x300B)
#define HOLDING_REG_SIZE      100     // 48个寄存器 (0x4000-0x4036)


//流速和流量
extern double g_flow_rate;
extern  double g_flow;

// 初始化Modbus数据
void init_modbus_data();

// 更新物理量到输入寄存器
void update_input_registers();

// 处理读线圈请求
void handle_read_coils(modbus_parser_t *parser);

// 处理写单个线圈请求
void handle_write_single_coil(modbus_parser_t *parser);

// 处理读保持寄存器请求
void handle_read_holding_registers(modbus_parser_t *parser);

// 处理写单个保持寄存器请求
void handle_write_single_register(modbus_parser_t *parser);

// 处理写多个保持寄存器请求
void handle_write_multiple_registers(modbus_parser_t *parser);

// 处理读输入寄存器请求
void handle_read_input_registers(modbus_parser_t *parser);

// 处理读离散输入请求
void handle_read_discrete_inputs(modbus_parser_t *parser);

// 在process_modbus_frame中添加功能码处理
void process_modbus_frame(modbus_parser_t *parser);

// 发送Modbus响应
void send_modbus_response(uint8_t address, uint8_t function, 
                         uint8_t *data, uint8_t data_len);

// 发送异常响应
void send_exception_response(modbus_parser_t *parser, uint8_t exception_code);
#endif