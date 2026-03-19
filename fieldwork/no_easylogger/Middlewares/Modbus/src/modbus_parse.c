#include "modbus_parse.h"
#include "bsp_usart.h"
#include "circular_buffer.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <string.h>

static circular_buf_t *g_modbus_parse_circular_buf;
QueueHandle_t g_modbus_parse_queue;

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
    // 使用 memset 将 parser 结构体的所有成员初始化为 0
    memset(&parser, 0, sizeof(modbus_parser_t));

    while (1)
    {
        // 获取当前时间戳
        uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

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

        // 处理接收到的数据
        // buffer_is_empty 返回 0x00 表示缓冲区不为空，可以继续处理数据
        while (buffer_is_empty(g_modbus_parse_circular_buf) == 0x00)
        {
            uint8_t byte = 0;
            if (buffer_get_data(g_modbus_parse_circular_buf, &byte) == 0x00)
            {
                // 成功获取一个字节的数据，继续处理
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
                        parser.state = MODBUS_STATE_ADDRESS;     // 转换到地址状态
                        parser.address = byte;                   // 存储接收到的地址字节
                        parser.last_char_time = current_time_ms; // 更新最后接收字符的时间戳
                    }

                    break;
                case MODBUS_STATE_ADDRESS:
                    // 处理地址字节
                    break;
                case MODBUS_STATE_FUNCTION:
                    // 处理功能码字节
                    break;
                case MODBUS_STATE_DATA:
                    // 处理数据字节
                    break;
                case MODBUS_STATE_CRC_LOW:
                    // 处理CRC低字节
                    break;
                case MODBUS_STATE_CRC_HIGH:
                    // 处理CRC高字节
                    break;
                default:
                    // 处理未知状态
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