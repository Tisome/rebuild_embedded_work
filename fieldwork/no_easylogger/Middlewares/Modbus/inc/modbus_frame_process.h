#ifndef __MODBUS_FRAME_PROCESS_H__
#define __MODBUS_FRAME_PROCESS_H__

#include <stdint.h>
#include "modbus_protocol.h"

/* 数据区大小 */
#define MODBUS_COIL_COUNT           16
#define MODBUS_DISCRETE_INPUT_COUNT 16
#define MODBUS_HOLDING_REG_COUNT    64
#define MODBUS_INPUT_REG_COUNT      64

/* 初始化和更新 */
void init_modbus_data(void);
void update_input_registers(void);

/* 功能处理 */
void handle_read_coil(modbus_parser_t *parser);
void handle_read_discrete_inputs(modbus_parser_t *parser);
void handle_read_holding_registers(modbus_parser_t *parser);
void handle_read_input_registers(modbus_parser_t *parser);
void handle_write_single_coil(modbus_parser_t *parser);
void handle_write_single_registers(modbus_parser_t *parser);
void handle_write_multiple_registers(modbus_parser_t *parser);

void process_modbus_frame(modbus_parser_t *parser);

/* 响应 */
void modbus_send_response(uint8_t address, uint8_t function, uint8_t *data, uint8_t data_len);
void modbus_send_expection_respense(modbus_parser_t *parser, uint8_t expection_code);

/* 对外数据区 */
extern uint8_t g_modbus_coils[(MODBUS_COIL_COUNT + 7) / 8];
extern uint8_t g_modbus_discrete_inputs[(MODBUS_DISCRETE_INPUT_COUNT + 7) / 8];
extern uint16_t g_modbus_holding_registers[MODBUS_HOLDING_REG_COUNT];
extern uint16_t g_modbus_input_registers[MODBUS_INPUT_REG_COUNT];

extern QueueHandle_t g_modbus_cmd_queue;

#endif