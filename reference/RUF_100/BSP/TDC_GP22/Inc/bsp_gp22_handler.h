/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_gp22_handler.h
 * 
 * @par dependencies 
 * - bsp_gp22_driver.h
 * - elog.h
 * - i2c.h
 * - stm32f1xx_hal.h
 * @author rnz | Rvision
 * 
 * @brief Provide the HAL APIs of TDC-GP22 and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-8-6
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/
#include "bsp_gp22_driver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#ifndef BSP_GP22_HANDLER_H
#define BSP_GP22_HANDLER_H

extern gp22_data_t      g_GP22_data;

#define GP22_CALLBACK        void gp22_gpio_exti_handler(uint16_t GPIO_Pin)
/* GP22处理程序输入参数 */
typedef struct 
{
    spi_driver_interface_t             *spi_if;       /* SPI接口 */
    delay_interface_t                *delay_if;      /* 延时接口 */
    timebase_interface_t          *timebase_if;    /* 时间戳接口 */
    os_interface_t                      *os_if;       /* OS接口 */
    yield_interface_t                *yield_if;  /* 任务切换接口 */
} gp22_handler_input_args_t;  

/* GP22处理程序结构体 */
  typedef struct 
{
    spi_driver_interface_t             *spi_if;       /* SPI接口 */
    delay_interface_t                *delay_if;      /* 延时接口 */
    timebase_interface_t          *timebase_if;    /* 时间戳接口 */
    os_interface_t                      *os_if;       /* OS接口 */

    yield_interface_t                *yield_if;   /* 任务切换接口 */
    bsp_gp22_driver_t                   driver;   /* GP22驱动实例 */
    QueueHandle_t                    cmd_queue;       /* 命令队列 */     
    SemaphoreHandle_t                    mutex;         /* 互斥锁 */
    TaskHandle_t                   task_handle;       /* 任务句柄 */
    uint32_t                         queue_len;       /* 队列长度 */
    uint32_t                         item_size;     /* 队列项大小 */
} bsp_gp22_handler_t;

/* 初始化处理程序 */
gp22_status_t gp22_handler_init
(
    bsp_gp22_handler_t                *handler,
    gp22_handler_input_args_t      *input_args,
    uint32_t                         queue_len,
    uint32_t                         item_size
);

/* 处理程序任务函数 */
void gp22_handler_thread(void *argument);

/* 从队列获取测量数据 */
gp22_status_t gp22_handler_get_data
(
    bsp_gp22_handler_t                 *handler,
    gp22_data_t                           *data,
    uint32_t                            timeout
);

/* 发送命令到处理程序 */
gp22_status_t gp22_handler_send_cmd
(
    bsp_gp22_handler_t                 *handler,
    uint8_t                                 cmd,
    uint32_t                             timeout
);

typedef enum {
    GP22_CMD_NONE          =    0,
    GP22_CMD_START_TOF     =    1,
    GP22_CMD_START_TEMP    =    2,
    GP22_CMD_CALIBRATE     =    3,
    GP22_CMD_GET_DATA      =    4,    
} gp22_cmd_t;   

extern gp22_data_t  g_GP22_data;
extern bsp_gp22_handler_t *g_gp22_handler;

gp22_status_t gp22_start_meature_tof(void);
gp22_status_t gp22_get_result_tof(void); 
gp22_status_t gp22_app_modify_reg_attr(uint8_t reg,
                                       uint32_t attr_mask,
                                       uint8_t new_val);
#endif /* BSP_GP22_HANDLER_H */
