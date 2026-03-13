#ifndef __MID_CIRCULAR_BUFFER__
#define __MID_CIRCULAR_BUFFER__

#include <stdint.h>

#define CIRCULAR_BUFFER_SIZE 100

typedef uint8_t data_type_t;

typedef struct {
    data_type_t  data[CIRCULAR_BUFFER_SIZE];
    uint32_t                           head;
    uint32_t                           tail;
}circular_buffer_t;
extern circular_buffer_t * g_circular_buffer_irq_thread;
circular_buffer_t * create_empty_circular_buffer (void);

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
uint8_t             buffer_is_empty  (circular_buffer_t * p_buffer);
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
uint8_t             buffer_is_full   (circular_buffer_t * p_buffer);
/**
 * @brief insert_data.
 * 
 *  
 * @param[in] circular_buffer_t : Pointer to the target of handler.
 * 
* @return      uint8_t : 
                        0xff:error, the buffer is full;
                        0x00:success
                        0x01:failed
 * 
 * */
uint8_t             insert_data      (circular_buffer_t * p_buffer,\
                                                  data_type_t data);
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
                                                data_type_t * data);
                                                
                                                
                                                
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
                                                    uint32_t * head);

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
                                            uint32_t increament_num);

#endif // end __MID_CIRCULAR_BUFFER__