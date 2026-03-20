#include "modbus_frame_process.h"
#include "modbus_crc.h"
#include "modbus_map.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "data.h"

#include "usart.h"

#include "RS485.h"

#include "elog.h"

#include <string.h>

/* -------------------- Modbus 数据区 -------------------- */

extern Pipe_Parameters_t g_parameters;

/* bit区按位打包 */
uint8_t g_modbus_coils[(MODBUS_COIL_COUNT + 7) / 8];
uint8_t g_modbus_discrete_inputs[(MODBUS_DISCRETE_INPUT_COUNT + 7) / 8];

/* 16位寄存器区 */
uint16_t g_modbus_holding_registers[MODBUS_HOLDING_REG_COUNT];
uint16_t g_modbus_input_registers[MODBUS_INPUT_REG_COUNT];

QueueHandle_t g_modbus_cmd_queue;

/* -------------------- 函数声明 -------------------- */
void modbus_send_response(uint8_t address,
                          uint8_t function,
                          uint8_t *data,
                          uint8_t data_len);

void modbus_send_expection_respense(modbus_parser_t *parser,
                                    uint8_t expection_code);

/* -------------------- 内部辅助函数 -------------------- */

static void set_s32_to_regs(uint16_t *regs, uint16_t start_idx, int32_t value)
{
    uint32_t u = (uint32_t)value;
    regs[start_idx] = (uint16_t)((u >> 16) & 0xFFFF);
    regs[start_idx + 1] = (uint16_t)(u & 0xFFFF);
}

static void set_s64_to_regs(uint16_t *regs, uint16_t start_idx, int64_t value)
{
    uint64_t u = (uint64_t)value;
    regs[start_idx] = (uint16_t)((u >> 48) & 0xFFFF);
    regs[start_idx + 1] = (uint16_t)((u >> 32) & 0xFFFF);
    regs[start_idx + 2] = (uint16_t)((u >> 16) & 0xFFFF);
    regs[start_idx + 3] = (uint16_t)(u & 0xFFFF);
}

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
// 这里的每个置位都要有相应的执行
static void handle_write_single_coil(modbus_parser_t *parser)
{
    uint16_t addr = modbus_get_u16_be(&parser->data[0]);
    uint16_t value = modbus_get_u16_be(&parser->data[2]);
    modbus_cmd_t cmd = MODBUS_CMD_NONE;

    if (addr >= MODBUS_COIL_COUNT)
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }

    if ((value != 0xFF00) && (value != 0x0000))
    {
        modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }

    /* 1. 状态型 coil：直接保存状态 */
    if (addr == MB_COIL_MEASURE_ENABLE)
    {
        set_bit_to_buf(g_modbus_coils, addr, (value == 0xFF00) ? 1U : 0U);

        modbus_send_response(parser->address,
                             parser->function,
                             parser->data,
                             4);
        return;
    }

    /* 2. 命令型 coil：只接受写1触发 */
    if (value == 0xFF00)
    {
        switch (addr)
        {
        case MB_COIL_CLEAR_TOTALIZER:
            cmd = MODBUS_CMD_CLEAR_TOTALIZER;
            break;

        case MB_COIL_ZERO_LEARN_START:
            cmd = MODBUS_CMD_ZERO_LEARN_START;
            break;

        case MB_COIL_SAVE_PARAMETERS:
            cmd = MODBUS_CMD_SAVE_PARAMETERS;
            break;

        case MB_COIL_LOAD_DEFAULT_PARAMETERS:
            cmd = MODBUS_CMD_LOAD_DEFAULT_PARAMETERS;
            break;

        case MB_COIL_CLEAR_ALARM:
            cmd = MODBUS_CMD_CLEAR_ALARM;
            break;

        case MB_COIL_SOFT_RESET:
            cmd = MODBUS_CMD_SOFT_RESET;
            break;

        default:
            modbus_send_expection_respense(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
            return;
        }
        // 如果是软件复位的话，那么先发一个返回的值给modbus上位机，不然上位机无法接收到这个返回的数据
        if (cmd == MODBUS_CMD_SOFT_RESET)
        {
            modbus_send_response(parser->address,
                                 parser->function,
                                 parser->data,
                                 4);
        }

        if (xQueueSend(g_modbus_cmd_queue, &cmd, 0) != pdPASS)
        {
            modbus_send_expection_respense(parser, MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY);
            return;
        }

        /* 命令触发型：可以短暂置1后立刻清0，也可以直接不保留 */
        set_bit_to_buf(g_modbus_coils, addr, 0U);
    }
    else
    {
        /* 对命令型 coil 写0，一般直接清0即可 */
        set_bit_to_buf(g_modbus_coils, addr, 0U);
    }

    /* 0x05 响应是回显原请求的 addr + value */
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
    g_modbus_cmd_queue = xQueueCreate(8, sizeof(modbus_cmd_t));
    if (g_modbus_cmd_queue == NULL)
    {
        log_e("Failed to create queue for USART IRQ");
        vTaskDelete(NULL); // 删除当前任务
    }

    memset(g_modbus_coils, 0, sizeof(g_modbus_coils));
    memset(g_modbus_discrete_inputs, 0, sizeof(g_modbus_discrete_inputs));
    memset(g_modbus_holding_registers, 0, sizeof(g_modbus_holding_registers));
    memset(g_modbus_input_registers, 0, sizeof(g_modbus_input_registers));

    update_holding_registers_from_parameters();
    update_input_registers();
}
// 更新input 包括寄存器和离散输入
void update_input_registers(void)
{
    int32_t s32_val = 0;
    int64_t s64_val = 0;

    /* 1. 清空输入寄存器 */
    memset(g_modbus_input_registers, 0, sizeof(g_modbus_input_registers));

    /* 2. 清空离散输入 */
    memset(g_modbus_discrete_inputs, 0, sizeof(g_modbus_discrete_inputs));

    /* =========================
     * 实时输出区：g_algo_out
     * ========================= */

    /* flow_speed, unit: m/s, scale x1000 */
    s32_val = (int32_t)(g_algo_out.flow_speed * 1000.0);
    set_s32_to_regs(g_modbus_input_registers, IR_FLOW_SPEED_H, s32_val);

    /* flow_rate_instant, scale x1000 */
    s32_val = (int32_t)(g_algo_out.flow_rate_instant * 1000.0);
    set_s32_to_regs(g_modbus_input_registers, IR_FLOW_RATE_INSTANT_H, s32_val);

    /* flow_rate_total, scale x1000 */
    s64_val = (int64_t)(g_algo_out.flow_rate_total * 1000.0);
    set_s64_to_regs(g_modbus_input_registers, IR_FLOW_RATE_TOTAL_H, s64_val);

    /* sq_value, scale x1000 */
    s32_val = (int32_t)(g_algo_out.sq_value * 1000.0);
    set_s32_to_regs(g_modbus_input_registers, IR_SQ_VALUE_H, s32_val);

    /* =========================
     * 诊断区：g_algo_state
     * ========================= */
    g_modbus_input_registers[IR_ZERO_STABLE] = g_algo_state.zero_stable;
    g_modbus_input_registers[IR_SQ_IDX] = g_algo_state.sq_idx;
    g_modbus_input_registers[IR_SQ_COUNT] = g_algo_state.sq_count;
    g_modbus_input_registers[IR_SQ_BAD_COUNT] = g_algo_state.sq_bad_count;
    g_modbus_input_registers[IR_WINDOW_IDX] = g_algo_state.window_idx;
    g_modbus_input_registers[IR_STEP_CNT] = g_algo_state.step_cnt;
    g_modbus_input_registers[IR_WINDOW_FULL] = g_algo_state.window_full ? 1U : 0U;

    /* q_total_m3, scale x1000 */
    s64_val = (int64_t)(g_algo_state.q_total_m3 * 1000.0);
    set_s64_to_regs(g_modbus_input_registers, IR_Q_TOTAL_M3_H, s64_val);

    /* =========================
     * 离散输入：状态 / 告警
     * ========================= */

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ALARM_EXIST,
                   (g_alarm != ALARM_OK) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ALARM_REPEAT_PACKET,
                   (g_alarm == ALARM_REPEAT_PACKET) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ALARM_OUT_OF_TIME,
                   (g_alarm == ALARM_OUT_OF_TIME) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ALARM_SPEED_LOWER,
                   (g_alarm == ALARM_SPEED_LOWER_LIMIT) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ALARM_SPEED_HIGHER,
                   (g_alarm == ALARM_SPEED_HIGHER_LIMIT) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ALARM_RATE_LOW,
                   (g_alarm == ALARM_RATE_TOO_LOW) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ALARM_RATE_HIGH,
                   (g_alarm == ALARM_RATE_TOO_HIGH) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_ZERO_STABLE_REACHED,
                   (g_algo_state.zero_stable >= g_parameters.zero_stable_threshold) ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_WINDOW_FULL,
                   g_algo_state.window_full ? 1U : 0U);

    set_bit_to_buf(g_modbus_discrete_inputs, DI_PARAMETER_SAVED,
                   g_parameters.is_saved ? 1U : 0U);

    /* 当前测量值有效：这里先简单定义为“无报警” */
    set_bit_to_buf(g_modbus_discrete_inputs, DI_MEASUREMENT_VALID,
                   (g_alarm == ALARM_OK) ? 1U : 0U);
}

// 更新holding_register，也就是parameter中的参数
void update_holding_registers_from_parameters(void)
{
    memset(g_modbus_holding_registers, 0, sizeof(g_modbus_holding_registers));

    g_modbus_holding_registers[0] = (uint16_t)(g_parameters.inner_diameter * 100.0);
    g_modbus_holding_registers[1] = (uint16_t)(g_parameters.wall_thick * 100.0);

    set_s32_to_regs(g_modbus_holding_registers, 2,
                    (int32_t)(g_parameters.cos_value * 1000000.0));

    set_s32_to_regs(g_modbus_holding_registers, 4,
                    (int32_t)(g_parameters.sin_value * 1000000.0));

    set_s32_to_regs(g_modbus_holding_registers, 6,
                    (int32_t)(g_parameters.lower_speed_range * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 8,
                    (int32_t)(g_parameters.upper_speed_range * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 10,
                    (int32_t)(g_parameters.alarm_lower_rate_range * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 12,
                    (int32_t)(g_parameters.alarm_upper_rate_range * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 14,
                    (int32_t)(g_parameters.zero_offset_speed * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 16,
                    (int32_t)(g_parameters.zero_learn_flow_speed * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 18,
                    (int32_t)(g_parameters.zero_learn_alpha * 1000000.0));

    set_s32_to_regs(g_modbus_holding_registers, 20,
                    (int32_t)(g_parameters.zero_learn_offset_max * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 22,
                    (int32_t)(g_parameters.zero_learn_sq_min * 1000.0));

    set_s32_to_regs(g_modbus_holding_registers, 24,
                    (int32_t)(g_parameters.te_ns));

    g_modbus_holding_registers[26] = (uint16_t)g_parameters.is_saved;
    g_modbus_holding_registers[27] = (uint16_t)g_parameters.output_mode;
    g_modbus_holding_registers[28] = (uint16_t)g_parameters.display_sensitivity;
    g_modbus_holding_registers[29] = (uint16_t)g_parameters.zero_stable_threshold;
    g_modbus_holding_registers[30] = (uint16_t)g_parameters.modbus_addr;
    g_modbus_holding_registers[31] = (uint16_t)g_parameters.pipe_type;
    g_modbus_holding_registers[32] = (uint16_t)g_parameters.speed_unit_type;
    g_modbus_holding_registers[33] = (uint16_t)g_parameters.rate_unit_type;
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
