/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_iwdg_handler.h
 * 
 * @par dependencies 
 * - iwdg.h
 * - elog.h
 * - i2c.h
 * - stm32f1xx_hal.h
 * @author rnz | Rvision
 * 
 * @brief Provide the HAL APIs of IWDG and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-8-21
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/
#ifndef BSP_IWDG_HANDLER_H
#define BSP_IWDG_HANDLER_H
//********************* Include *********************************************//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "bsp_iwdg_driver.h"

//********************* Marco ***********************************************//
#define WDT_MONITOR_WAIT_MS 50
#define WDT_INITED 1
#define WDT_NOT_INITED 0

#define WDT_MONITOR_TASK_MAX 10

#define WDT_TICK xTaskGetTickCount()

//********************* Typedef *********************************************//
typedef enum
{
    WDT_THREAD_A,
    WDT_THREAD_MAX,
} wdt_thread_type_t;

//********************* Interface Function **********************************//
iwdg_status_t wdt_monitor_init(void);

iwdg_status_t wdt_monitor_task_register(const char *name,
                                      uint32_t max_period,
                                        uint32_t *out_id);

iwdg_status_t wdt_monitor_task_enable(uint32_t id);

iwdg_status_t wdt_monitor_task_disable(uint32_t id);

iwdg_status_t wdt_monitor_task_period_set(uint32_t id, uint32_t max_period);

iwdg_status_t monitor_task_timeout_check(void);

iwdg_status_t wdt_monitor_task_feed(uint32_t id);

iwdg_status_t wdt_monitor_notify(void);

void system_hang_up(void);

#endif