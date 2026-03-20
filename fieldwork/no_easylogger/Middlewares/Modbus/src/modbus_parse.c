#include "modbus_parse.h"
#include "modbus_crc.h"
#include "modbus_protocol.h"

#include "bsp_usart.h"
#include "circular_buffer.h"

#include "elog.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <string.h>

static circular_buf_t *g_modbus_parse_circular_buf;
QueueHandle_t g_modbus_parse_queue;

// 使用 memset 将 parser 结构体的所有成员初始化为 0
// 同时设置状态为IDLE
void reset_modbus_parser(modbus_parser_t *parser)
{
    memset(parser, 0, sizeof(modbus_parser_t));
    parser->state = MODBUS_STATE_IDLE;
}

void task_modbus_parse(void *parameter)
{
    static uint32_t modbus_receive;

    // 创建队列，用于UART驱动任务给Modbus解析任务发送信息
    g_modbus_parse_queue = xQueueCreate(1, sizeof(uint32_t));
    if (g_modbus_parse_queue == NULL)
    {
        log_e("Failed to create queue for Modbus parse task");
        vTaskDelete(NULL); // 删除当前任务
    }

    // 获取 UART 驱动任务创建的循环缓冲区指针
    g_modbus_parse_circular_buf = get_uart_circular_buffer();
    if (g_modbus_parse_circular_buf == NULL)
    {
        log_e("Failed to get circular buffer for Modbus parse task");
        vTaskDelete(NULL); // 删除当前任务
    }

    // 初始化 Modbus 解析器状态
    modbus_parser_t parser;

    reset_modbus_parser(&parser);

    while (1)
    {

        // 等待 UART 驱动任务发送消息，表示有新的数据需要解析
        xQueueReceive(g_modbus_parse_queue, &modbus_receive, portMAX_DELAY);

        // 检查接收到的消息是否是预期的消息
        if (modbus_receive != UART_TASK_SEND_TO_MODBUS_TASK)
        {
            log_e("Received unknown message in Modbus parse task");
            continue; // 继续等待下一个消息
        }

        // 如果接受数据的指针为空，说明指针有问题
        if (g_modbus_parse_circular_buf == NULL)
        {
            log_e("Circular buffer is NULL in Modbus parse task");
            continue; // 继续等待下一个消息
        }

        uint32_t current_time_ms = 0;

        // 处理接收到的数据
        // buffer_is_empty 返回 0x00 表示缓冲区不为空，可以继续处理数据
        while (buffer_is_empty(g_modbus_parse_circular_buf) == 0x00)
        {
            uint8_t byte = 0;

            // 获取当前时间戳
            current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

            // 如果当前状态不是空闲，也就是正在处理
            // 同时超时了
            if (parser.state != MODBUS_STATE_IDLE &&
                (current_time_ms - parser.last_char_time) > pdMS_TO_TICKS(MODBUS_TIMEOUT_MS))
            {
                reset_modbus_parser(&parser);
            }

            // buffer_get_data返回0x00表示接收到一个数据
            if (buffer_get_data(g_modbus_parse_circular_buf, &byte) == 0x00)
            {
                // 不管当前是什么状态，都更新last_char_time
                parser.last_char_time = current_time_ms;
                // 根据 parser.state 的状态来解析 Modbus 数据
                switch (parser.state)
                {
                case MODBUS_STATE_IDLE:
                    // 处理空闲状态，等待地址字节
                    if (byte != MODBUS_SLAVE_ADDR)
                    {
                        log_e("Received byte does not match Modbus slave address, ignoring");
                        break; // 继续等待下一个字节
                    }
                    else
                    {
                        parser.state = MODBUS_STATE_ADDRESS_DONE__FUNCTION_START;
                        parser.address = byte;
                        parser.calculated_crc = 0xFFFF;
                        parser.calculated_crc = modbus_crc_update(parser.calculated_crc, byte);
                    }

                    break;
                case MODBUS_STATE_ADDRESS_DONE__FUNCTION_START:
                    // 地址字节已经被处理，现在在处理功能码
                    parser.function = byte;
                    parser.calculated_crc = modbus_crc_update(parser.calculated_crc, byte);
                    switch (parser.function)
                    {
                    // 这些功能码后面跟四字节
                    // 起始地址2字节 + 数量2字节
                    case (MODBUS_FUNC_READ_COILS):
                    case (MODBUS_FUNC_READ_DISCRETE_INPUTS):
                    case (MODBUS_FUNC_READ_HOLDING_REGISTERS):
                    case (MODBUS_FUNC_READ_INPUT_REGISTERS):
                        parser.expected_data_length = 4;
                        break;

                    // 这些功能码后面跟四字节
                    // 起始地址2字节 + 值2字节
                    case (MODBUS_FUNC_WRITE_SINGLE_COIL):
                    case (MODBUS_FUNC_WRITE_SINGLE_REGISTER):
                        parser.expected_data_length = 4;
                        break;

                    // 该功能码后面的格式是
                    // 起始地址2字节 + 寄存器数量2字节 + 要写入的字节数1字节 + N字节个数据
                    case (MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS):
                        parser.expected_data_length = 5;

                    default:
                        parser.expected_data_length = 0;
                        break;
                    }
                    parser.data_length = 0;
                    parser.state = MODBUS_STATE_FUNCTION_DONE__DATA_START;
                    break;
                case MODBUS_STATE_FUNCTION_DONE__DATA_START:
                    // 功能码字节已经被处理，现在在处理数据段
                    parser.data[parser.data_length++] = byte;
                    parser.calculated_crc = modbus_crc_update(parser.calculated_crc, byte);

                    // 判断多数据写入的情况
                    if (parser.function == MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS &&
                        parser.data_length == 5)
                    {
                        // 在功能码之后的数据结构是
                        // 起始地址2字节 + 寄存器数量2字节 + 要写入的字节数1字节
                        // 因此取出data[4]就是剩下的要写入的字节
                        // 这时候要增加expected_data_length的长度
                        uint8_t byte_cnt = parser.data[4];
                        parser.expected_data_length = byte_cnt + 5;
                    }

                    if (parser.data_length == parser.expected_data_length)
                    {
                        parser.state = MODBUS_STATE_DATA_DONE__CRC_LOW;
                    }

                    break;
                case MODBUS_STATE_DATA_DONE__CRC_LOW:
                    // 数据字节处理结束，开始接收CRC低字节
                    parser.crc = (uint16_t)byte;
                    parser.state = MODBUS_STATE_CRC_LOW_DONE__CRC_HIGH;
                    break;
                case MODBUS_STATE_CRC_LOW_DONE__CRC_HIGH:
                    // 处理CRC高字节
                    parser.crc |= (byte << 8);

                    if (parser.calculated_crc = parer.crc)
                    {
                        process_modbus_frame(&parser);
                    }
                    else
                    {
                        log_e("Error modbus crc");
                    }
                    reset_modbus_parser(&parser);

                    break;
                default:
                    // 处理未知状态
                    parser.state = MODBUS_STATE_IDLE;
                    break;
                }
            }
            else
            {
                log_e("Failed to get data from circular buffer in Modbus parse task");
                break; // 跳出循环，等待下一个消息
            }
        }
    }
}