/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_key_irq.c
 * 
 * @par dependencies 
 * - bsp_key.h
 * - task.h
 * - FreeRTOS.h
 * - queue.h
 * - semphr.h
 * 
 * @author rnz
 * 
 * @brief Provide the HAL APIs of Key and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-7-5
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/
#include "bsp_key.h"
#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//******************************** Defines **********************************//
#define FALING_TYPE 0
#define RASING_TYPE 1

static uint32_t irq_type[KEY_NUMS] = {FALING_TYPE}; 

typedef struct {
 uint32_t frist_trigger_tick; 
    uint8_t event_index;      
} key_state_t;


QueueHandle_t              key_queue = NULL;
QueueHandle_t        inter_key_queue = NULL;



//******************************** Defines **********************************//

#define KEY_PRESS_GET_TICK      xTaskGetTickCount()

/**
 * @brief key_task_func for thread to run.
 * 
 * Steps:
 *  1.doing the specific operations
 *  
 * @param[in] void 
 * 
 * @return led_handler_status_t : Status of the function.
 * 
 * */
void key_task_func(void *argument)
{
    key_state_t key_states[KEY_NUMS] = {0}; 
    /**     Variables (in task stack)                              **/
    uint32_t           short_press_time    =         SHORT_PRESS_TIME;
    uint32_t           key_time_out        =             KEY_TIME_OUT;
    key_press_event_t  key_press_event     =     
    { 
     .edge_type = RASING,
     .trigger_tick = 0
    };
    key_press_event_t* event;
    
    /**     Variables (in task stack)                              **/

    /**     Variables (in OS heap )                                **/
    key_queue       = xQueueCreate( 20, sizeof(uint16_t));
    inter_key_queue = xQueueCreate( 20, 4 );
    /**     Variables (in OS heap )                                **/

    // check if the queue has been created successfully
    if (NULL == key_queue     || 
        NULL == inter_key_queue)
    {
        log_d("key_queue created failed \r\n");
        return;
    } 
    else
    {
//        log_d("key_queue created successfully \r\n");
    }

    for(;;)
    {
//        log_d( "key_task_func running "
//                " at [%d] tick \r\n",  KEY_PRESS_GET_TICK);
        //1. check if there is new data about the key press in the queue
        if( pdTRUE == xQueueReceive(           inter_key_queue,  
                                          &event, 
                                           portMAX_DELAY ) )
        {
            key_state_t *state = &key_states[event->key_id];
            
            key_press_event.edge_type    =    event->edge_type;
            key_press_event.trigger_tick = event->trigger_tick;
            
            //1.1 if there is the new data about the key, 
            //then update it in state machine
            if( RASING  == key_press_event.edge_type &&
                0       ==                state->event_index)
            {
                state->frist_trigger_tick  =         0;
                state->event_index         =         0;
                log_e("key RASING fetched! error!");
            }
            if( FAILING == key_press_event.edge_type &&
                0       ==                state->event_index)
            {
//                log_e("key FAILING fetched! first\r\n");
                //chang the index for chaging the state machine
                state->event_index += 1;     

                //Mark the first tick when event coming
                state->frist_trigger_tick  = key_press_event.trigger_tick;
            }
            if( RASING  == key_press_event.edge_type &&
                1       ==                state->event_index )
            {
//                log_e("key RASING after the falling ");
                //1.1.1 if the interval in new key event between two key 
                // is less than 10ms, 
                if ( key_press_event.trigger_tick - state->frist_trigger_tick < 5)
                {
                    // 1.1.1.1 the new key press event is not valid
                    log_e( "Invalid key fetched "
                            " at [%d] tick \r\n",  KEY_PRESS_GET_TICK);
                    state->frist_trigger_tick = 0;
                    state->event_index = 0;
                    continue;
                }

                //1.1.2 if the interval in new key event between two key
                // is more than 10ms, then the key press event is valid 
                // 1.1.2.1 the new key press event is valid
                // 1.1.2.1.1 if the interval is less than the short_press time
                // then it should be short press.
                if ((key_press_event.trigger_tick - state->frist_trigger_tick) < short_press_time)
                {
                     key_press_status_t key_result = KEY_SHORT_PRESSED;
                     uint16_t final_key_value = (event->key_id << 8) | key_result;
                     if ( pdTRUE == xQueueSendToFront(        key_queue,
                                                           &final_key_value,
                                                           0))
                     {
                    //    log_d( "key_result send short press successfully"
                    //            " at [%d] tick \r\n", 
                    //                           KEY_PRESS_GET_TICK);
                     }
                     else
                     {
                        log_e("key_result send short press failed" 
                               "at [%d] tick \r\n", 
                                         KEY_PRESS_GET_TICK);
                     }
                }
                else if ((key_press_event.trigger_tick - state->frist_trigger_tick) < key_time_out)
                {
                     key_press_status_t key_result = KEY_LONG_PRESSED;
                     uint16_t final_key_value = (event->key_id << 8) | key_result;
                     if ( pdTRUE == xQueueSendToFront(        key_queue,
                                                           &final_key_value,
                                                           0))
                     {
                    //    log_d( "key_result send long press successfully"
                    //            " at [%d] tick \r\n", 
                    //                           KEY_PRESS_GET_TICK);
                     }
                     else
                     {
                    log_e("key_result send long press failed" 
                           "at [%d] tick \r\n", 
                                     KEY_PRESS_GET_TICK);
                     }
                }
                state->frist_trigger_tick = 0;
                state->event_index = 0;
            }
         }

    }
    /* USER CODE END 5 */
}

/**
 * @brief key_interuption_callback
 * 
 * Steps:
 *  1.if trigger first time with falling type,\
 *    send the event to the inter_key_queue \
 *    changing the interruption type to Raising
 *  
 *  2.if trigger first time with Raising type,\
 *    send the event to the inter_key_queue \
 *    changing the interruption type back to falling
 *  
 * @param[in] void 
 * 
 * @return void
 * 
 * */
KEY_CALLBACK 
{
    static key_press_event_t key_events[KEY_NUMS][2] = {0}; 
    static uint32_t last_trigger_tick[KEY_NUMS]      = {0};
    const uint32_t debounce_interval                 = DEBOUNCE_TIME;  
    uint32_t current_tick                            = KEY_PRESS_GET_TICK;
    
    
    key_press_event_t * p_key_press_1                  = NULL;
    key_press_event_t * p_key_press_2                  = NULL;
    
    
    static key_id_t key_id;
    switch(GPIO_Pin) {
        case KEY0_GPIO_PIN: key_id = KEY_ID_0; break;
        case KEY1_GPIO_PIN: key_id = KEY_ID_1; break;
        case KEY2_GPIO_PIN: key_id = KEY_ID_2; break;
        case KEY3_GPIO_PIN: key_id = KEY_ID_3; break;
        default: return; 
    }
    
    
    if (current_tick - last_trigger_tick[key_id] < DEBOUNCE_TIME) {
//        log_d("Debounce ignored: Key[%d] at %dms", key_id, current_tick);
        return;
    }
    last_trigger_tick[key_id] = current_tick;
   /*
    1.if trigger first time with falling type,\
      send the event to the inter_key_queue \
      changing the interruption type to Raising
    */
    BaseType_t xHigherPrioritTaskWoken;
    
    if ( FALING_TYPE == irq_type[key_id] )
    {
        if ( NULL == inter_key_queue )
        {
            
            
//            log_e( "inter_key_queue not created"
//                    " at [%d] tick \r\n", 
//                                   KEY_PRESS_GET_TICK);
        } 
//         log_d("keyid = %d\r\n",key_id);
         key_events[key_id][0] = (key_press_event_t){
            .key_id = key_id,
            .edge_type = FAILING,
            .trigger_tick = KEY_PRESS_GET_TICK
        };   
        p_key_press_1 = &key_events[key_id][0];
        if ( pdTRUE == xQueueSendFromISR(            inter_key_queue, 
                                                               &p_key_press_1, 
                                                    &xHigherPrioritTaskWoken ))
        {

//            log_e( "key_press_event[%d] send FALLING_event successfully"
//                    " at [%d] tick \r\n",p_key_press_1->key_id,
//                                   KEY_PRESS_GET_TICK);
        }
    /*
    1.1 changing the irq type
    */
        irq_type[key_id] = RASING_TYPE;
    /*
    1.2 changing the GPIO irq trigger type
    */ 
    }
    else if ( RASING_TYPE == irq_type[key_id] )
    {
            
    /*  
    2.if trigger first time with Raising type,\
      send the event to the inter_key_queue \
      changing the interruption type back to falling
    */

    
        if ( NULL == inter_key_queue )
        {
//            log_e( "inter_key_queue not created"
//                    " at [%d] tick \r\n", 
//                                   KEY_PRESS_GET_TICK);
        }
        
         key_events[key_id][1] = (key_press_event_t){
            .key_id = key_id,
            .edge_type = RASING,
            .trigger_tick = KEY_PRESS_GET_TICK
        }; 
        p_key_press_2 = &key_events[key_id][1];
        if ( pdTRUE == xQueueSendFromISR(              inter_key_queue, 
                                                                 &p_key_press_2, 
                                                     &xHigherPrioritTaskWoken ))
        {
//            log_e( "key_press_event[%d] send RASING_event successfully"
//                    " at [%d] tick \r\n",p_key_press_2->key_id,
//                                   KEY_PRESS_GET_TICK);
        }
        
    /*
    1.1 changing the irq type
    */
        irq_type[key_id] = FALING_TYPE;
    /*
    1.2 changing the GPIO irq trigger type
    */ 
    }
    portYIELD_FROM_ISR(xHigherPrioritTaskWoken);
}


int key_scan(char mode) 
{
    uint16_t raw_value;
    BaseType_t ret = xQueueReceive(key_queue, &raw_value, mode ? portMAX_DELAY : 0);
    if(ret == pdTRUE) {
        key_id_t key_id = raw_value >> 8;
        key_press_status_t status = raw_value & 0xFF;
        switch(key_id) {
            case KEY_ID_0: 
                return (status == KEY_SHORT_PRESSED) ? KEY0_PRES : KEY0_LONG_PRESS;
            case KEY_ID_1:
                return (status == KEY_SHORT_PRESSED) ? KEY1_PRES : KEY1_LONG_PRESS;
             case KEY_ID_2: 
                return (status == KEY_SHORT_PRESSED) ? KEY2_PRES : KEY2_LONG_PRESS;
            case KEY_ID_3:
                return (status == KEY_SHORT_PRESSED) ? KEY3_PRES : KEY3_LONG_PRESS;
        }
    }
    return KEY_NONE;
}




