/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_iwdg_handler.c
 * 
 * @par dependencies 
 * - bsp_iwdg_handler.h
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
//********************* Include *********************************************//
#include "main.h"
#include "bsp_iwdg_handler.h"

//********************* Marco ***********************************************//

//********************* Typedef *********************************************//
typedef struct
{
    const char *name;
    uint32_t max_period_tick;
    uint32_t last_update;
    uint8_t enable;
} monitor_task_t;

typedef struct
{
    bool inited;
    SemaphoreHandle_t lock;
    TaskHandle_t thread;
    // st_drv_wdt_channel_id channel_id;
    uint32_t task_num;
    monitor_task_t tasks[WDT_MONITOR_TASK_MAX];
} wdt_monitor_data_t;

//********************* Declare *********************************************//

wdt_monitor_data_t wdt_data = 
{
    .inited = false,
    .lock = NULL,
    .thread = NULL,
    .task_num = 0,
};

//********************* Internal Function ***********************************//
/**
 * @brief hang up thread goto offline break point wait for debugger 
 * @param void NULL
 * @retval return result
 */
void system_hang_up(void)
{

    /************* wait for IWDG trigge ******************/
    while (1)
    {
        CoreDebug->DEMCR |= 1 << 16U;
        __asm__("bkpt #1")
    }
}

/**
 * @brief check monited thread overflow
 * @param void NULL
 * @retval bool
 */
iwdg_status_t monitor_task_timeout_check(void)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check init state***********************/
    if (WDT_INITED != wdt_data.inited)
    {
        log_e("[%s] wdt monitor not init\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    /************* take thread lock **********************/
    if (pdTRUE != xSemaphoreTake(wdt_data.lock, portMAX_DELAY))
    {
        log_e("[%s] wdt take lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_BUSY;
        return ret;
    }

    uint32_t current_time = WDT_TICK;

    /************* check monited thread overflow *********/
    for (uint32_t i = 0; i < wdt_data.task_num; i++)
    {
        uint32_t tmp_inteval;
        if(WDT_TICK > wdt_data.tasks[i].last_update)
        {
            tmp_inteval = WDT_TICK - wdt_data.tasks[i].last_update;
        }
        else
        {
            tmp_inteval =                                  WDT_TICK + 
                      (0xFFFFFFFF - wdt_data.tasks[i].last_update);
        }
        if ((wdt_data.tasks[i].enable) &&
            (tmp_inteval >
             wdt_data.tasks[i].max_period_tick))
        {
            log_e("Last Tick = %d,Current Tick = %d,Task %s timeout!!!", 
                                 wdt_data.tasks[i].last_update,WDT_TICK,
                                                wdt_data.tasks[i].name);
            ret = IWDT_ERROR_BUSY;
        }
    }

    /************* give thread lock **********************/
    xSemaphoreGive(wdt_data.lock);

    return ret;
}

//********************* Interface Function **********************************//
/**
 * @brief initialize the wdt module
 * @param void NULL
 * @retval return result
 */
iwdg_status_t wdt_monitor_init(void)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check reinit **************************/
    if (WDT_INITED == wdt_data.inited)
    {
        log_e("[%s] wdt monitor reinit\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    /************* create thread locker ******************/
    if (NULL == wdt_data.lock)
        wdt_data.lock = xSemaphoreCreateMutex();

    if (NULL == wdt_data.lock)
    {
        log_e("[%s] create lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_NO_MEM;
        return ret;
    }

    /************* take thread lock **********************/
    if (pdTRUE != xSemaphoreTake(wdt_data.lock, portMAX_DELAY))
    {
        log_e("[%s] wdt take lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_BUSY;
        return ret;
    }

    /************* hardware initial **********************/
    bsp_iwdt_init();

    bsp_iwdt_enable();

    wdt_data.inited = true;

    /************* give thread lock **********************/
    xSemaphoreGive(wdt_data.lock);

    return ret;
}

/**
 * @brief registe thread monitor
 * @param name monited thread name
 * @param max_period max timeout term(ms)
 * @param out_id monited thread id (storage in monited thread)
 * @retval return result
 */
iwdg_status_t wdt_monitor_task_register(const char *name,
                                     uint32_t max_period,
                                     uint32_t *out_id)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check param **************************/
    if ((NULL == name) || (NULL == out_id))
    {
        log_e(" [%s] invalid param !!! \r\n", __FUNCTION__);
        return IWDT_ERROR_INVALID_PARAM;
    }

    /************* check init state***********************/
    if (WDT_INITED != wdt_data.inited)
    {
        log_e("[%s] wdt monitor not init\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    /************* take thread lock **********************/
    if (pdTRUE != xSemaphoreTake(wdt_data.lock, portMAX_DELAY))
    {
        log_e("[%s] wdt take lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_BUSY;
        return ret;
    }

    /************* register thread monitor ***************/
    wdt_data.tasks[wdt_data.task_num].name = name;
    wdt_data.tasks[wdt_data.task_num].max_period_tick = max_period;
    wdt_data.tasks[wdt_data.task_num].enable = false;
    wdt_data.tasks[wdt_data.task_num].last_update = WDT_TICK;

    *out_id = wdt_data.task_num;
    wdt_data.task_num++;

    /************* give thread lock **********************/
    xSemaphoreGive(wdt_data.lock);

    return ret;
}

/**
 * @brief enable thread monitor
 * @param id monited thread id (storage in monited thread)
 * @retval return result
 */
iwdg_status_t wdt_monitor_task_enable(uint32_t id)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check param **************************/
    if (id >= WDT_MONITOR_TASK_MAX)
    {
        log_e(" [%s] invalid param !!! \r\n", __FUNCTION__);
        return IWDT_ERROR_INVALID_PARAM;
    }

    /************* check init state***********************/
    if (WDT_INITED != wdt_data.inited)
    {
        log_e("[%s] wdt monitor not init\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    /************* take thread lock **********************/
    if (pdTRUE != xSemaphoreTake(wdt_data.lock, portMAX_DELAY))
    {
        log_e("[%s] wdt take lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_BUSY;
        return ret;
    }

    /************* register monited thread ***************/
    wdt_data.tasks[id].enable = true;
    wdt_data.tasks[id].last_update = WDT_TICK;

    /************* give thread lock **********************/
    xSemaphoreGive(wdt_data.lock);

    return ret;
}

/**
 * @brief disable thread monitor
 * @param id monited thread id (storage in monited thread)
 * @retval return result
 */
iwdg_status_t wdt_monitor_task_disable(uint32_t id)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check param **************************/
    if (id >= WDT_MONITOR_TASK_MAX)
    {
        log_e(" [%s] invalid param !!! \r\n", __FUNCTION__);
        return IWDT_ERROR_INVALID_PARAM;
    }

    /************* check init state***********************/
    if (WDT_INITED != wdt_data.inited)
    {
        log_e("[%s] wdt monitor not init\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    /************* take thread lock **********************/
    if (pdTRUE != xSemaphoreTake(wdt_data.lock, portMAX_DELAY))
    {
        log_e("[%s] wdt take lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_BUSY;
        return ret;
    }

    /************* register monited thread ***************/
    wdt_data.tasks[id].enable = false;

    /************* give thread lock **********************/
    xSemaphoreGive(wdt_data.lock);

    return ret;
}

/**
 * @brief disable thread monitor
 * @param id monited thread id (storage in monited thread)
 * @param max_period max timeout term(ms)
 * @retval return result
 */
iwdg_status_t wdt_monitor_task_period_set(uint32_t id, uint32_t max_period)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check param **************************/
    if (id >= WDT_MONITOR_TASK_MAX)
    {
        log_e(" [%s] invalid param !!! \r\n", __FUNCTION__);
        return IWDT_ERROR_INVALID_PARAM;
    }

    /************* check init state***********************/
    if (WDT_INITED != wdt_data.inited)
    {
        log_e("[%s] wdt monitor not init\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    /************* take thread lock **********************/
    if (pdTRUE != xSemaphoreTake(wdt_data.lock, portMAX_DELAY))
    {
        log_e("[%s] wdt take lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_BUSY;
        return ret;
    }

    /************* register thread monitor ***************/
    wdt_data.tasks[id].max_period_tick = max_period;

    /************* give thread lock **********************/
    xSemaphoreGive(wdt_data.lock);

    return ret;
}

/**
 * @brief feed thread monitor
 * @param id monited thread id (storage in monited thread)
 * @retval return result
 */
iwdg_status_t wdt_monitor_task_feed(uint32_t id)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check param **************************/
    if (id >= WDT_MONITOR_TASK_MAX)
    {
        log_e(" [%s] invalid param !!! \r\n", __FUNCTION__);
        return IWDT_ERROR_INVALID_PARAM;
    }

    /************* check init state***********************/
    if (WDT_INITED != wdt_data.inited)
    {
        log_e("[%s] wdt monitor not init\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    /************* take thread lock **********************/
    if (pdTRUE != xSemaphoreTake(wdt_data.lock, portMAX_DELAY))
    {
        log_e("[%s] wdt take lock fail !!! \r\n", __FUNCTION__);
        ret = IWDT_ERROR_BUSY;
        return ret;
    }

    /************* register thread monitor ***************/
    wdt_data.tasks[id].last_update = WDT_TICK % 0xFFFFFF;

    /************* give thread lock **********************/
    xSemaphoreGive(wdt_data.lock);

    return ret;
}

/**
 * @brief notify monitor thread
 * @param void NULL
 * @retval return result
 */
iwdg_status_t wdt_monitor_notify(void)
{
    iwdg_status_t ret = IWDT_SUCCESS;

    /************* check init state***********************/
    if (WDT_INITED != wdt_data.inited)
    {
        log_e("[%s] wdt monitor not init\r\n", __FUNCTION__);
        ret = IWDT_ERROR_INVALID_STATE;
        return ret;
    }

    xTaskNotifyGive(wdt_data.thread);

    return ret;
}
