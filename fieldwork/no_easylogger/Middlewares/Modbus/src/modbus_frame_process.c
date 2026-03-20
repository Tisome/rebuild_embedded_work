#include "modbus_frame_process.h"
#include "modbus_crc.h"

#include "data.h"

#include "usart.h"

#include "RS485.h"

#include "elog.h"

#include <string.h>

/* -------------------- Modbus 数据区 -------------------- */

/* bit区按位打包 */
uint8_t g_modbus_coils[(MODBUS_COIL_COUNT + 7) / 8];
uint8_t g_modbus_discrete_inputs[(MODBUS_DISCRETE_INPUT_COUNT + 7) / 8];

/* 16位寄存器区 */
uint16_t g_modbus_holding_registers[MODBUS_HOLDING_REG_COUNT];
uint16_t g_modbus_input_registers[MODBUS_INPUT_REG_COUNT];

/* -------------------- 函数声明 -------------------- */
void modbus_send_response(uint8_t address,
                          uint8_t function,
                          uint8_t *data,
                          uint8_t data_len);

void modbus_send_expection_respense(modbus_parser_t *parser,
                                    uint8_t expection_code);

/* -------------------- 内部辅助函数 -------------------- */

static uint8_t get_bit_from_buf(const uint8_t *buf, uint16_t bit_index)
{
    return (buf[bit_index / 8] >> (bit_index % 8)) & 0x01;
}

static void set_bit_to_buf(uint8_t *buf, uint16_t bit_index, uint8_t value)
{
    if (value)
    {
        buf[bit_index / 8] |= (1U << (bit_index % 8));
    }
    else
    {
        buf[bit_index / 8] &= ~(1U << (bit_index % 8));
    }
}

// 高字节在前，低字节在后
static uint16_t modbus_get_u16_be(const uint8_t *buf)
{
    return ((uint16_t)buf[0] << 8) | buf[1];
}

// buf[0]放高字节
static void modbus_put_u16_be(uint8_t *buf, uint16_t value)
{
    buf[0] = (uint8_t)((value >> 8) & 0xFF);
    buf[1] = (uint8_t)(value & 0xFF);
}

/* -------------------- 读线圈 0x01 -------------------- */

// 可以连续读线圈，返回的值在data中连续给出
static void handle_read_coil(modbus_parser_t *parser)
{
    uint16_t start_addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t quantity = modbus_get_u16_be(&parser->data[2]);

    if ((quantity < 1) || (quantity > 2000))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    if ((start_addr + quantity) > MODBUS_COIL_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    uint8_t byte_count = (quantity + 7) / 8;
    uint8_t response[1 + 250];

    response[0] = byte_count;
    memset(&response[1], 0, byte_count);

    for (uint16_t i = 0; i < quantity; i++)
    {
        if (get_bit_from_buf(g_modbus_coils, start_addr + i))
        {
            response[1 + i / 8] |= (1U << (i % 8));
        }
    }

    modbus_send_response(parser->address,
                         parser->function,
                         response,
                         (uint8_t)(1 + byte_count));
}

/* -------------------- 读离散输入 0x02 -------------------- */

static void handle_read_discrete_inputs(modbus_parser_t *parser)
{
    uint16_t start_addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t quantity = modbus_get_u16_be(&parser->data[2]);

    if ((quantity < 1) || (quantity > 2000))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    if ((start_addr + quantity) > MODBUS_DISCRETE_INPUT_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    uint8_t byte_count = (quantity + 7) / 8;
    uint8_t response[1 + 250];

    response[0] = byte_count;
    memset(&response[1], 0, byte_count);

    for (uint16_t i = 0; i < quantity; i++)
    {
        if (get_bit_from_buf(g_modbus_discrete_inputs, start_addr + i))
        {
            response[1 + i / 8] |= (1U << (i % 8));
        }
    }

    modbus_send_response(parser->address,
                         parser->function,
                         response,
                         (uint8_t)(1 + byte_count));
}

/* -------------------- 读保持寄存器 0x03 -------------------- */

static void handle_read_holding_registers(modbus_parser_t *parser)
{
    uint16_t start_addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t quantity = modbus_get_u16_be(&parser->data[2]);

    if ((quantity < 1) || (quantity > 125))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    if ((start_addr + quantity) > MODBUS_HOLDING_REG_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    uint8_t response[1 + 250];
    response[0] = (uint8_t)(quantity * 2);

    for (uint16_t i = 0; i < quantity; i++)
    {
        modbus_put_u16_be(&response[1 + i * 2],
                          g_modbus_holding_registers[start_addr + i]);
    }

    modbus_send_response(parser->address,
                         parser->function,
                         response,
                         (uint8_t)(1 + quantity * 2));
}

/* -------------------- 读输入寄存器 0x04 -------------------- */

static void handle_read_input_registers(modbus_parser_t *parser)
{
    uint16_t start_addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t quantity = modbus_get_u16_be(&parser->data[2]);

    if ((quantity < 1) || (quantity > 125))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    if ((start_addr + quantity) > MODBUS_INPUT_REG_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    update_input_registers();

    uint8_t response[1 + 250];
    response[0] = (uint8_t)(quantity * 2);

    for (uint16_t i = 0; i < quantity; i++)
    {
        modbus_put_u16_be(&response[1 + i * 2], g_modbus_input_registers[start_addr + i]);
    }

    modbus_send_response(parser->address,
                         parser->function,
                         response,
                         (uint8_t)(1 + quantity * 2));
}

/* -------------------- 写单个线圈 0x05 -------------------- */

// 0xFF00 置位
// 0x0000 清零
static void handle_write_single_coil(modbus_parser_t *parser)
{
    uint16_t coil_addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t coil_value = modbus_get_u16_be(&parser->data[2]);

    if (coil_addr >= MODBUS_COIL_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    if ((coil_value != 0x0000) && (coil_value != 0xFF00))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    set_bit_to_buf(g_modbus_coils, coil_addr, (coil_value == 0xFF00) ? 1 : 0);

    /* 协议要求：正常响应回显原请求 */
    modbus_send_response(parser->address,
                         parser->function,
                         parser->data,
                         4);
}

/* -------------------- 写单个寄存器 0x06 -------------------- */

static void handle_write_single_registers(modbus_parser_t *parser)
{
    uint16_t reg_addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t reg_value = modbus_get_u16_be(&parser->data[2]);

    if (reg_addr >= MODBUS_HOLDING_REG_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    g_modbus_holding_registers[reg_addr] = reg_value;

    /* 协议要求：正常响应回显原请求 */
    modbus_send_response(parser->address,
                         parser->function,
                         parser->data,
                         4);
}

/* -------------------- 写多个寄存器 0x10 -------------------- */

static void handle_write_multiple_registers(modbus_parser_t *parser)
{
    uint16_t start_addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t quantity = modbus_get_u16_be(&parser->data[2]);
    uint8_t byte_count = parser->data[4];

    if ((quantity < 1) || (quantity > 123))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    if (byte_count != (quantity * 2))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    if ((start_addr + quantity) > MODBUS_HOLDING_REG_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    for (uint16_t i = 0; i < quantity; i++)
    {
        g_modbus_holding_registers[start_addr + i] =
            modbus_get_u16_be(&parser->data[5 + i * 2]);
    }

    /* 响应：起始地址 + 数量 */
    uint8_t response[4];
    modbus_put_u16_be(&response[0], start_addr);
    modbus_put_u16_be(&response[2], quantity);

    modbus_send_response(parser->address,
                         parser->function,
                         response,
                         4);
}

/* -------------------- 响应发送 -------------------- */

static void modbus_send_response(uint8_t address,
                                 uint8_t function,
                                 uint8_t *data,
                                 uint8_t data_len)
{
    uint8_t frame[260];
    uint16_t crc = 0xFFFF;
    uint16_t frame_len = 0;

    frame[frame_len++] = address;
    frame[frame_len++] = function;

    for (uint16_t i = 0; i < data_len; i++)
    {
        frame[frame_len++] = data[i];
    }

    for (uint16_t i = 0; i < frame_len; i++)
    {
        crc = modbus_crc_update(crc, frame[i]);
    }

    /* Modbus RTU: CRC低字节在前 */
    frame[frame_len++] = (uint8_t)(crc & 0xFF);
    frame[frame_len++] = (uint8_t)((crc >> 8) & 0xFF);

    // 在传输之前先打开RS485的
    HAL_GPIO_WritePin(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_GPIO_PIN, GPIO_PIN_SET);
    usart1_send_bytes(frame, frame_len);

    // TC标志表示 整个帧已经发完
    while (__HAL_USART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET)
        ;
    HAL_GPIO_WritePin(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_GPIO_PIN, GPIO_PIN_RESET);
}

/* -------------------- 异常响应 -------------------- */

static void modbus_send_expection_respense(modbus_parser_t *parser,
                                           uint8_t expection_code)
{
    uint8_t data[1];

    /* 异常响应功能码 = 原功能码 | 0x80 */
    data[0] = expection_code;

    modbus_send_response(parser->address,
                         (uint8_t)(parser->function | 0x80),
                         data,
                         1);
}

/* -------------------- 初始化 -------------------- */

void init_modbus_data(void)
{
    rs485_init();

    memset(g_modbus_coils, 0, sizeof(g_modbus_coils));
    memset(g_modbus_discrete_inputs, 0, sizeof(g_modbus_discrete_inputs));
    memset(g_modbus_holding_registers, 0, sizeof(g_modbus_holding_registers));
    memset(g_modbus_input_registers, 0, sizeof(g_modbus_input_registers));

    /* 示例初始化，可按你的业务改 */
    g_modbus_holding_registers[0] = 0x1234;
    g_modbus_holding_registers[1] = 0x5678;

    g_modbus_input_registers[0] = 100;
    g_modbus_input_registers[1] = 200;

    set_bit_to_buf(g_modbus_discrete_inputs, 0, 1); /* 设备正常 */
}

/* 把实时业务值刷到输入寄存器 */
void update_input_registers(void)
{
    /* 这里按你的实际传感器数据更新 */
    /* 示例 */
    g_modbus_input_registers[0] += 1;
    g_modbus_input_registers[1] += 2;
}

/* -------------------- 功能分发 -------------------- */

void process_modbus_frame(modbus_parser_t *parser)
{
    if (parser->address != MODBUS_SLAVE_ADDR)
    {
        log_e("Modbus address mismatch");
        return;
    }

    switch (parser->function)
    {
    case MODBUS_FUNC_READ_COILS:
        handle_read_coil(parser);
        break;

    case MODBUS_FUNC_READ_DISCRETE_INPUTS:
        handle_read_discrete_inputs(parser);
        break;

    case MODBUS_FUNC_READ_HOLDING_REGISTERS:
        handle_read_holding_registers(parser);
        break;

    case MODBUS_FUNC_READ_INPUT_REGISTERS:
        handle_read_input_registers(parser);
        break;

    case MODBUS_FUNC_WRITE_SINGLE_COIL:
        handle_write_single_coil(parser);
        break;

    case MODBUS_FUNC_WRITE_SINGLE_REGISTER:
        handle_write_single_registers(parser);
        break;

    case MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS:
        handle_write_multiple_registers(parser);
        break;

    default:
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
        break;
    }
}
