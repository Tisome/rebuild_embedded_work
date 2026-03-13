/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_gp22_driver.h
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
 

//******************************** Includes *********************************//
#include "bsp_gp22_handler.h"
#include "bsp_gp22_reg.h"   
#include "elog.h"
#include <string.h>
#include "task_manager.h"
#include "emit.h"

gp22_data_t            g_GP22_data;
bsp_gp22_handler_t *g_gp22_handler;


gp22_status_t gp22_handler_init(bsp_gp22_handler_t *handler,
                      gp22_handler_input_args_t *input_args,
                      uint32_t queue_len,uint32_t item_size) 
{
    if (NULL == handler || NULL == input_args   || 
                     NULL == input_args->spi_if || 
                     NULL == input_args->os_if) 
    {
        return GP22_ERR_PARAM;
    }

    memset(handler, 0, sizeof(bsp_gp22_handler_t));
    handler->spi_if                    =                input_args->spi_if;
    handler->delay_if                  =              input_args->delay_if;
    handler->timebase_if               =           input_args->timebase_if;
    handler->os_if                     =                 input_args->os_if;
    handler->yield_if                  =              input_args->yield_if;
              
    handler->queue_len                 =                         queue_len;
    handler->item_size                 =                         item_size;

    if (handler->os_if->os_semaphore_create_mutex((void *)&handler->mutex)
                                                               != GP22_OK) 
    {
        log_e("Failed to create mutex for GP22 handler");
        return GP22_ERROR;
    }

    if (handler->os_if->
        os_queue_create(queue_len, sizeof(gp22_cmd_t), 
                          (void *)&handler->cmd_queue) 
                                           != GP22_OK) 
    {
        handler->os_if->os_semaphore_delete_mutex(handler->mutex);
        return GP22_ERROR;
    }

    if (bsp_gp22_driver_inst(   &handler->driver,handler->spi_if,
    handler->delay_if,handler->timebase_if,handler->spi_if->hspi) 
                                                      != GP22_OK) 
    {
        return GP22_ERROR;
    }

    return GP22_OK;
}

void gp22_handler_thread(void *argument) 
{
    gp22_handler_input_args_t *input = (gp22_handler_input_args_t *)argument;
    if (NULL == input) 
    {
        log_e("bsp_handler_thread argument is NULL");
        return;
    }
    bsp_gp22_handler_t *handler      = (bsp_gp22_handler_t *)
                                        malloc(sizeof(bsp_gp22_handler_t));

    g_gp22_handler   = handler;
    gp22_cmd_t             cmd;
    gp22_status_t          ret;


    //初始化handler
    memset(handler, 0, sizeof(bsp_gp22_handler_t));
    gp22_handler_init(handler, input, 10, sizeof(gp22_data_t));

    //log_d("GP22 Handler Thread Start");
    while (1) 
    {
        // 等待命令
        if (handler->os_if->os_queue_get(handler->cmd_queue, &cmd, portMAX_DELAY) 
                                                                      != GP22_OK) 
        {
            continue;
        }
        handler->os_if->os_semaphore_lock_mutex(handler->mutex);

        switch (cmd) {
            case GP22_CMD_START_TOF:
                ret = handler->driver.start_tof(&handler->driver);
                log_d("Tick = %d",handler->timebase_if->pf_get_tick_count());
                if (ret == GP22_OK) 
                {
                    //sweep_start();
                }
                else
                {
                    log_e("Failed to start TOF measurement: %d", ret);
                }
                break;
            case GP22_CMD_START_TEMP:
                ret = handler->driver.start_temp(&handler->driver);
                if (ret != GP22_OK) 
                {   
                    log_e("Failed to  start temperature measurement: %d", ret);
                    handler->os_if->os_semaphore_unlock_mutex(handler->mutex);
                }
                break;
            case GP22_CMD_CALIBRATE:
                handler->driver.calibrate_clk(&handler->driver);
                handler->driver.calibrate_tdc(&handler->driver);
                break;
            case GP22_CMD_GET_DATA:
                handler->driver.get_measure_data(&handler->driver, &g_GP22_data);
                break;
            default:
                break;
        }
        handler->os_if->os_semaphore_unlock_mutex(handler->mutex);
        
    }
}

gp22_status_t gp22_handler_send_cmd(bsp_gp22_handler_t *handler,
                                   uint8_t cmd,uint32_t timeout) 
{
    if (!handler || cmd >= GP22_CMD_GET_DATA) 
    {
        return GP22_ERR_PARAM;
    }
    return handler->os_if->os_queue_put(handler->cmd_queue, &cmd, timeout);
}


// gp22_status_t gp22_start_meature_tof(void) 
// {
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     gp22_cmd_t cmd = GP22_CMD_START_TOF;
//     if (g_gp22_handler->os_if->os_queue_put_isr(g_gp22_handler->cmd_queue, &cmd,
//                                                       &xHigherPriorityTaskWoken) 
//                                                                      != GP22_OK) 
//     {
//         log_e("Failed to send test command to GP22 handler");
//         return GP22_ERROR;
//     }
//     return GP22_OK;
// }
gp22_status_t gp22_start_meature_tof(void) 
{
    gp22_status_t ret;
    ret = g_gp22_handler->driver.start_tof(&g_gp22_handler->driver);
    if (ret == GP22_OK) 
    {
        sweep_start();
    }
    else
    {
        log_e("Failed to start TOF measurement: %d", ret);
    }
    gp22_get_result_tof();
    return ret;
}


gp22_status_t  gp22_get_result_tof(void) 
{
    gp22_status_t ret;
    if(NULL == g_gp22_handler)
    {
        log_d("GP22 handler is NULL");
        ret = GP22_ERROR;
        return ret;
    }
    ret = g_gp22_handler->driver.get_measure_data(&g_gp22_handler->driver, 
                                                            &g_GP22_data);
    if (ret != GP22_OK) 
    {
        log_e("Failed to start TOF measurement: %d", ret);
    }
    return ret;
}
/**
 * @brief TDC-GP22 INT interrupt callback
 * 
 * @param[in]
 * 
 * @return void
*/

GP22_CALLBACK
{
    // SEGGER_SYSVIEW_RecordEnterISR();
    // gp22_get_result_tof();
    // SEGGER_SYSVIEW_RecordExitISR(); 
}


gp22_status_t gp22_app_modify_reg_attr(uint8_t reg,
                                       uint32_t attr_mask,
                                       uint8_t new_val)
{
    __disable_irq();
    gp22_status_t   ret = GP22_OK;
    g_gp22_handler->spi_if->cs_select(0); 
    ret = g_gp22_handler->spi_if->write_reg(g_gp22_handler->driver.spi_handle, 
                                                            GP22_WRITE_REG_CMD | reg, GP22_EN_INT_BIT(0)| GP22_DELVAL1_BIT(attr_mask));
    // ret = g_gp22_handler->spi_if->write_cmd(g_gp22_handler->driver.spi_handle, 
    //                                                             GP22_INIT_CMD);
    // ret = g_gp22_handler->spi_if->write_cmd(g_gp22_handler->driver.spi_handle,
    //                                                        GP22_START_TOF_CMD);
    g_gp22_handler->spi_if->cs_select(1);
    __enable_irq();
    // for(int i = 0; i < 100; i++) __asm volatile("nop");
    return ret;
}
 //******************************** Functions ********************************//