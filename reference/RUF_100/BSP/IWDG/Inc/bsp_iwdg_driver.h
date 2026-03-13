/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_iwdg_driver.h
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
#ifndef BSP_IWDG_DRIVER_H
#define BSP_IWDG_DRIVER_H
#include "iwdg.h"
#include "elog.h"

//********************* Include *********************************************//

#include <stdint.h>


//********************* Typedef *********************************************//
typedef enum
{
    IWDT_SUCCESS,
    IWDT_ERROR_INVALID_STATE,
    IWDT_ERROR_NO_MEM,
    IWDT_ERROR_BUSY,
    IWDT_ERROR_RESOURCES,
    IWDT_ERROR_INVALID_PARAM,

} iwdg_status_t;

//********************* Interface Function **********************************//

iwdg_status_t bsp_iwdt_init(void);

iwdg_status_t bsp_iwdt_enable(void);

iwdg_status_t bsp_iwdt_feed(void);

#endif