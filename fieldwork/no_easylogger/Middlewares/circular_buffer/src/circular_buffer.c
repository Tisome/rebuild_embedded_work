#include "circular_buffer.h"
#include "elog.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

circular_buf_t *create_empty_circular_buffer(void)
{
    circular_buf_t *p_buffer = NULL;
    p_buffer = (circular_buf_t *)malloc(sizeof(circular_buf_t));
    if (p_buffer != NULL)
    {
        memset(p_buffer, 0, sizeof(circular_buf_t));
    }
    else
    {
        log_e("Failed to allocate memory for circular buffer");
        return NULL;
    }
    return p_buffer;
}

uint8_t buffer_is_null(circular_buf_t *p_buffer)
{
    return (p_buffer == NULL) ? 0x01 : 0x00;
}

uint8_t buffer_is_empty(circular_buf_t *p_buffer)
{
    if (buffer_is_null(p_buffer))
    {
        return 0xFF;
    }
    // 如果count为0，说明缓冲区没有数据，即为空，也就是输出0x01；否则输出0x00
    return (p_buffer->count == 0) ? 0x01 : 0x00;
}

uint8_t buffer_is_full(circular_buf_t *p_buffer)
{
    if (buffer_is_null(p_buffer))
    {
        return 0xFF;
    }
    // 如果count等于缓冲区大小，说明缓冲区已满，输出0x01；否则输出0x00
    return (p_buffer->count == CIRCULAR_BUF_SIZE) ? 0x01 : 0x00;
}

uint32_t buffer_get_count(circular_buf_t *p_buffer)
{
    if (buffer_is_null(p_buffer))
    {
        return 0xFFFFFFFF; // 返回一个特殊值表示错误
    }
    return p_buffer->count;
}

uint8_t buffer_push(circular_buf_t *p_buffer, circular_buf_data_t data)
{
    if (buffer_is_null(p_buffer))
        return 0xFF;

    if (buffer_is_full(p_buffer))
        return 0xFE;

    p_buffer->buffer[p_buffer->head] = data;

    p_buffer->head = (p_buffer->head + 1) % CIRCULAR_BUF_SIZE;

    p_buffer->count++;

    return 0x00;
}

uint8_t buffer_pop(circular_buf_t *p_buffer, circular_buf_data_t *data)
{
    if (buffer_is_null(p_buffer))
        return 0xFF;

    if (buffer_is_empty(p_buffer))
        return 0xFE;

    *data = p_buffer->buffer[p_buffer->tail];

    p_buffer->tail = (p_buffer->tail + 1) % CIRCULAR_BUF_SIZE;

    p_buffer->count--;

    return 0x00;
}

uint8_t buffer_clear(circular_buf_t *p_buffer)
{
    if (buffer_is_null(p_buffer))
        return 0xFF;

    p_buffer->head = 0;
    p_buffer->tail = 0;
    p_buffer->count = 0;
    return 0x00;
}

uint8_t buffer_peek(circular_buf_t *p_buffer, circular_buf_data_t *data)
{
    if (buffer_is_null(p_buffer))
        return 0xFF;

    if (buffer_is_empty(p_buffer))
        return 0xFE;

    *data = p_buffer->buffer[p_buffer->tail];

    return 0x00;
}