#ifndef __CIRCULAR_BUF_H
#define __CIRCULAR_BUF_H

#include <stdint.h>

#define CIRCULAR_BUF_SIZE 128

typedef uint8_t circular_buf_data_t;

typedef struct {
    circular_buf_data_t buffer[CIRCULAR_BUF_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} circular_buf_t;

/**
 * @brief Check if the circular buffer pointer is NULL
 *
 * @param p_buffer Pointer to the circular buffer
 * @return uint8_t :
 * 0x00: p_buffer is not NULL
 * 0x01: p_buffer is NULL
 */
uint8_t buffer_is_null(circular_buf_t *p_buffer);

/**
 * @brief Check if the circular buffer is empty
 *
 * @param p_buffer Pointer to the circular buffer
 * @return uint8_t :
 * 0x00: p_buffer is not empty
 * 0x01: p_buffer is empty
 * 0xFF: p_buffer is NULL/ERROR
 */
uint8_t buffer_is_empty(circular_buf_t *p_buffer);

/**
 * @brief Check if the circular buffer is full
 *
 * @param p_buffer Pointer to the circular buffer
 * @return uint8_t :
 * 0x00: p_buffer is not full
 * 0x01: p_buffer is full
 * 0xFF: p_buffer is NULL/ERROR
 */
uint8_t buffer_is_full(circular_buf_t *p_buffer);

/**
 * @brief Get the number of elements in the circular buffer
 *
 * @param p_buffer Pointer to the circular buffer
 * @return uint32_t : number of elements in the buffer
 */
uint32_t buffer_get_count(circular_buf_t *p_buffer);

/**
 * @brief Push data into the circular buffer
 *
 * @param p_buffer Pointer to the circular buffer
 * @param data Data to be pushed
 * @return uint8_t :
 * 0x00: Success
 * 0xFF: p_buffer is NULL/ERROR
 * 0xFE: Buffer is full
 */
uint8_t buffer_push(circular_buf_t *p_buffer, circular_buf_data_t data);

/**
 * @brief Pop data from the circular buffer
 *
 * @param p_buffer Pointer to the circular buffer
 * @param data Pointer to store the popped data
 * @return uint8_t :
 * 0x00: Success
 * 0xFF: p_buffer is NULL/ERROR
 * 0xFE: Buffer is empty
 */
uint8_t buffer_pop(circular_buf_t *p_buffer, circular_buf_data_t *data);

/**
 * @brief Clear the circular buffer
 *
 * @param p_buffer Pointer to the circular buffer
 * @return uint8_t :
 * 0x00: Success
 * 0xFF: p_buffer is NULL/ERROR
 */
uint8_t buffer_clear(circular_buf_t *p_buffer);

/**
 * @brief Peek at the data at the front of the circular buffer
 *
 * @param p_buffer Pointer to the circular buffer
 * @param data Pointer to store the peeked data
 * @return uint8_t :
 * 0x00: Success
 * 0xFF: p_buffer is NULL/ERROR
 * 0xFE: Buffer is empty
 */
uint8_t buffer_peek(circular_buf_t *p_buffer, circular_buf_data_t *data);

#endif /* __CIRCULAR BUF_H */