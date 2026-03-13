/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_iwdg_driver.c
 * 
 * @par dependencies 
 * - bsp_iwdg_driver.h
 * - main.h
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
#include "bsp_iwdg_driver.h"
#include "main.h"
//********************* extern prototype ************************************//

//********************* function detail *************************************//
/**
 * @brief feed the watchdog
 * @retval return result
 */
iwdg_status_t bsp_iwdt_feed(void)
{
    fwdgt_counter_reload();

    return IWDT_SUCCESS;
}

/**
 * @brief enable IWDG 
 * @retval return result
 */
iwdg_status_t bsp_iwdt_enable(void)
{
    fwdgt_enable();
    return IWDT_SUCCESS;
}

/**
 * @brief initialize IWDG
 * cubemx alreay finished hardware initialization
 * detal in function MX_IWDG_Init
 * @retval return result
 */
iwdg_status_t bsp_iwdt_init(void)
{
    if (fwdgt_config(4095, FWDGT_PSC_DIV32) != SUCCESS)
    {
        log_e("IWDG init error");
        return IWDT_ERROR_INVALID_STATE;
    }
    return IWDT_SUCCESS;
}