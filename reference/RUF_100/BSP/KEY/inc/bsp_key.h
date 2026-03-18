/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bsp_key_irq.h
 *
 * @par dependencies
 * - stdio.h
 * - stdint.h
 *
 * @author Jack | R&D Dept. | EternalChip 立芯嵌入式
 *
 * @brief Provide the HAL APIs of Key and corresponding opetions.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2023-12-03
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_KEY_IRQ_H__
#define __BSP_KEY_IRQ_H__

//******************************** Includes *********************************//

#include <stdint.h> //  the compiling lib.
#include <stdio.h>

#include "main.h" //  Core / OS layer

#include "queue.h" //  specific file for operations

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define KEY_NUMS         (4)

#define KEY_NONE         0 /* 没有按键按下 */
#define KEY0_PRES        1 /* KEY0按下 */
#define KEY1_PRES        2 /* KEY1按下 */
#define KEY2_PRES        3 /* KEY2按下 */
#define KEY3_PRES        4 /* KEY3按下 */
#define KEY0_LONG_PRESS  5 /* KEY0长按 */
#define KEY1_LONG_PRESS  6 /* KEY1长按 */
#define KEY2_LONG_PRESS  7 /* KEY2长按 */
#define KEY3_LONG_PRESS  8 /* KEY3长按 */

#define SHORT_PRESS_TIME (1000)  // Specify the short press time
#define KEY_TIME_OUT     (10000) // Specify the short press time
#define DEBOUNCE_TIME    (20)
#define KEY_CALLBACK     void key_gpio_exti_handler(uint16_t GPIO_Pin)

//*********************Thread_Func **********************//
extern TaskHandle_t key_TaskHandle;
extern const TaskHandle_t key_Task_attributes;
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
extern QueueHandle_t key_queue;
extern QueueHandle_t inter_key_queue;
//*********************Queue_Handler ********************//

typedef enum {
    KEY_OK             = 0,         /* Operation completed successfully.  */
    KEY_ERROR          = 1,         /* Run-time error without case matched*/
    KEY_ERRORTIMEOUT   = 2,         /* Operation failed with timeout      */
    KEY_ERRORRESOURCE  = 3,         /* Resource not available.            */
    KEY_ERRORPARAMETER = 4,         /* Parameter error.                   */
    KEY_ERRORNOMEMORY  = 5,         /* Out of memory.                     */
    KEY_ERRORISR       = 6,         /* Not allowed in ISR context         */
    KEY_RESERVED       = 0x7FFFFFFF /* Reserved                           */
} key_status_t;

typedef enum {
    KEY_PRESSED       = 0, /* Operation completed successfully.  */
    KEY_NOT_PRESSED   = 1, /* Run-time error without case matched*/
    KEY_SHORT_PRESSED = 2, /* Operation                          */
    KEY_LONG_PRESSED  = 3, /* Operation                          */
} key_press_status_t;

typedef enum {
    RASING  = 0, /* Operation completed successfully.  */
    FAILING = 1, /* Operation failed                   */
} key_trigger_edge_t;

// 添加按键枚举类型
typedef enum {
    KEY_ID_0 = 0,
    KEY_ID_1 = 1,
    KEY_ID_2 = 2,
    KEY_ID_3 = 3
} key_id_t;

typedef struct
{
    key_id_t key_id;              // 按键ID
    key_trigger_edge_t edge_type; /* Operation completed successfully.  */
    uint32_t trigger_tick;        /* Operation completed successfully.  */
} key_press_event_t;

//******************************** Defines **********************************//

//******************************** Declaring ********************************//
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
void key_task_func(void *argument);
int key_scan(char mode);
//******************************** Declaring ********************************//

#endif // End of __BSP_KEY_IRQ_H__
