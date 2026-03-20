#include "bsp_usart.h"
#include "circular_buffer.h"
#include "freertos_resources.h"

#include "modbus_parse.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "elog.h"
#define LOG_TAG "uart_driver"
#define LOG_LVL ELOG_LVL_VERBOSE

circular_buf_t *g_uart_circular_irq_buf;

static QueueHandle_t g_uart_irq_queue;

circular_buf_t *get_uart_circular_buffer(void)
{
    return g_uart_circular_irq_buf;
}

void task_uart_driver(void *parameter)
{
    uint32_t uart_receive;

    // 创建循环缓冲区，用于存储USART中断接收的数据
    g_uart_circular_irq_buf = create_empty_circular_buffer();

    if (g_uart_circular_irq_buf == NULL)
    {
        log_e("Failed to create circular buffer for USART");
        vTaskDelete(NULL); // 删除当前任务
    }

    // 创建队列，用于硬件串口给系统发送信息
    g_uart_irq_queue = xQueueCreate(1, sizeof(uint32_t));
    if (g_uart_irq_queue == NULL)
    {
        log_e("Failed to create queue for USART IRQ");
        vTaskDelete(NULL); // 删除当前任务
    }

    // 初始化USART1，使用DMA和IDLE中断
    uart1_dma_init(g_uart_circular_irq_buf->buffer, CIRCULAR_BUF_SIZE);

    while (1)
    {
        xQueueReceive(g_uart_irq_queue, &uart_receive, portMAX_DELAY);

        if (uart_receive == IRQ_SEND_TO_UART_DRIVER_TASK)
        {
            // 处理UART接收事件
            uint32_t send_to_end = UART_TASK_SEND_TO_MODBUS_TASK;
            BaseType_t queue_ret = xQueueGenericSend(g_modbus_parse_queue,
                                                     &send_to_end,
                                                     0,
                                                     queueOVERWRITE);
            if (queue_ret != pdTRUE)
            {
                log_e("ERROR, uart driver send to modbus task fail");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// 该uart中断回调函数接受当前dma写到的缓冲区位置，并把这个位置同步给软件层
// 然后发送数据给uart驱动任务，让它去处理接收到的数据
// 所以本质上这个函数还是给硬件/中断用的
void uart1_idle_irq_callback(uint16_t pos)
{
    if (pos >= CIRCULAR_BUF_SIZE)
    {
        log_e("Invalid position in UART idle IRQ callback: %u", pos);
        return;
    }
    buffer_change_head(g_uart_circular_irq_buf, pos);

    uint32_t send_to_uart_driver = IRQ_SEND_TO_UART_DRIVER_TASK;
    BaseType_t queue_ret = xQueueGenericSendFromISR(g_uart_irq_queue,
                                                    &send_to_uart_driver,
                                                    NULL,
                                                    queueOVERWRITE);
    if (queue_ret != pdPASS)
    {
        log_e("Failed to send message to UART driver task from ISR");
    }
}

// 这个回调函数先只是把DMA写指针的位置同步一下
// 后续可以添加其他功能
void uart1_dma_half_irq_callback()
{
    buffer_change_head(g_uart_circular_irq_buf, CIRCULAR_BUF_SIZE / 2);
}

// 这个回调函数先只是把DMA写指针的位置同步一下
// 后续可以在这里直接发送队列给uart_driver_task
// 但是这需要看后续modbus_task能不能做到一段命令没接受完的时候就开始处理了
void uart1_dma_full_irq_callback()
{
    buffer_change_head(g_uart_circular_irq_buf, 0);
}