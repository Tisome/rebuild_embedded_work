#include "mid_circular_buffer.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "elog.h"

//TBD : comment: create 
circular_buffer_t * create_empty_circular_buffer (void)
{
    circular_buffer_t * p_buffer_temp = NULL;
    // 1. alloct the memory space
    p_buffer_temp = (circular_buffer_t *) malloc(sizeof(circular_buffer_t));
    if ( NULL == p_buffer_temp ) 
    {
        log_e("error: create_empty_circular_buffer");
        return NULL;
    }
    
    // 2. memory init
    memset ( p_buffer_temp, 0, sizeof(circular_buffer_t));
    
    return p_buffer_temp;
}
/**
 * @brief buffer_is_empty.
 * 
 *  
 * @param[in] circular_buffer_t : Pointer to the target of handler.
 * 
* @return      uint8_t : 
                        0xff:error, the p_buffer is NULL;
                        0x00:is empty
                        0x01:is not empty
 * 
 * */
uint8_t             buffer_is_empty  (circular_buffer_t * p_buffer)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }        
    if ( p_buffer->head == p_buffer->tail )
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }        
}
/**
 * @brief buffer_is_full.
 * 
 *  
 * @param[in] circular_buffer_t : Pointer to the target of handler.
 * 
* @return      uint8_t : 
                        0xff:error, the p_buffer is NULL;
                        0x00:is full
                        0x01:is not full
 * 
 * */
uint8_t             buffer_is_full   (circular_buffer_t * p_buffer)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }  
    if (
        (( (p_buffer->head)%CIRCULAR_BUFFER_SIZE) + 1) == 
        ( (p_buffer->tail)%CIRCULAR_BUFFER_SIZE)
        )
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }      
}

/**
 * @brief insert_data.
 * 
 *  
 * @param[in] circular_buffer_t : Pointer to the target of handler.
 * 
* @return      uint8_t : 
                        0xff:error, the buffer pointer is NULL;
                        0xfe:error, the buffer is full;
                        0x00:success
                        0x01:failed
 * 
 * */
uint8_t             insert_data      (circular_buffer_t * p_buffer,\
                                                  data_type_t data)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }
    if( 0x00 == buffer_is_full(p_buffer) )
    {
        return 0xFE;
    }
    
    p_buffer->data[(p_buffer->head)%CIRCULAR_BUFFER_SIZE] = data;
    
    p_buffer->head++;
    
    return 0x00;


}

/**
 * @brief get_data.
 * 
 *  
 * @param[in] circular_buffer_t : Pointer to the target of handler.
 * 
* @return      uint8_t : 
                        0xff:error, the buffer pointer is NULL;
                        0xfe:error, the buffer is empty;
                        0x00:success
                        0x01:failed
 * 
 * */
uint8_t             get_data         (circular_buffer_t * p_buffer,\
                                                data_type_t * data)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }
    if( 0x00 == buffer_is_empty(p_buffer) )
    {
        return 0xFE;
    }
    *data = p_buffer->data[(p_buffer->tail)%CIRCULAR_BUFFER_SIZE];
    p_buffer->tail++;
    
    return 0x00;
}



/**
 * @brief get_the_head_pos.
 * 
 *  
 * @param[in] circular_buffer_t : Pointer to the target of handler.
 * @param[in] head : Pointer to the head storage varibale.
 * 
 * @return      uint8_t : 
                        0xff:error, the buffer pointer is NULL;
                        0xfe:error, the buffer is empty;
                        0x00:success
                        0x01:failed
 * 
 * */
uint8_t             get_head_pos   (circular_buffer_t * p_buffer,\
                                                    uint32_t * head)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }
    *head = p_buffer->head;
    
    return 0x00;
}


/**
 * @brief head pos. increment.
 * 
 *  
 * @param[in] circular_buffer_t : Pointer to the target of handler.
 * @param[in] increament_num : increment number.
* @return      uint8_t : 
                        0xff:error, the buffer pointer is NULL;
                        0xfe:error, the buffer is empty;
                        0x00:success
                        0x01:failed
 * 
 * */
uint8_t  head_pos_increment         (circular_buffer_t * p_buffer,\
                                            uint32_t increament_num)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }
    p_buffer->head += increament_num;
    
    return 0x00;
}
