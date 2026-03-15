/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bsp_at24c512_handler.h
 *
 * @par dependencies
 * - bsp_at24c512_handler.h
 * - elog.h
 * - stdlib.h
 * - free_rtos.h
 * @author rnz | Rvision
 *
 * @brief Provide the HAL APIs of AT24C512 and corresponding opetions.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2025-7-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_at24c512_handler.h"
#include "elog.h"
#include "freertos.h"
#include "stdlib.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define E2PROM_NOT_INITIATED false
#define E2PROM_INITIATED true

#define OS_QUEUE_CREATE handler_instance->os_interface->os_queue_create

#define MY_MAX_DELAY 0xffffffffUL
//******************************** Defines **********************************//

//******************************** Variables ********************************//

// Flag to indicate if the handler has been initialized
static bsp_at24c512_handler_t *gp_at24c512_instance = NULL;

//******************************** Variables ********************************//

//******************************** Functions ********************************//

/**
 * @brief Initialize the at24c512 handler.
 *
 * @param[in] handler_instance Pointer to the handler instance.
 *
 * @retval E2PROM_OK            Initialization successful.
 * @retval E2PROM_OK_ERRORRESOURCE Resource allocation or queue creation failed.
 */
static e2prom_status_t bsp_at24c512_handler_init(
    bsp_at24c512_handler_t *handler_instance)
{
    // log_d("bsp_at24c512_handler_init start");
    int8_t ret = 0;

    // Create event queue
    if (NULL == OS_QUEUE_CREATE)
    {
        log_e("os_interface is NULL");
        return E2PROM_ERRORRESOURCE;
    }

    ret = OS_QUEUE_CREATE(30, sizeof(e2prom_event_t),
                          &(handler_instance->event_queue_handle));
    // log_d("OS_QUEUE_CREATE ret=%d\r\n", ret);
    if (ret)
    {
#ifdef AT24C512_HANDLER_DEBUG
        log_e("error: create queue failed!");
#endif
        return E2PROM_ERRORRESOURCE;
    }

    // Initialize AT24C512 instance
    ret = at24c512_inst(handler_instance->pat24c512_instance,
                        handler_instance->iic_driver_interface,
#ifdef OS_SUPPORTING
                        handler_instance->yield_interface,
#endif // End of OS_SUPPORTING
                        handler_instance->timebase_interface);
#ifdef AT24C512_HANDLER_DEBUG
#endif
    if (ret)
    {
        log_e("at24c512_inst failed");
        return E2PROM_ERRORRESOURCE;
    }

    return E2PROM_OK;
}

/**
 * @brief De-initialize the at24c512 handler.
 *
 * @return E2PROM_OK De-initialization successful.
 */
// static e2prom_status_t bsp_at24c512_handler_deinit(void)
// {

//     return E2PROM_OK;
// }

/**
 * @brief Instantiate the at24c512 handler.
 *
 * @param[in] handler_instance Pointer to the handler instance.
 * @param[in] input_arg Pointer to the input arguments containing required
 *                      interfaces.
 *
 * @retval E2PROM_OK            Initialization successful.
 * @retval E2PROM_ERRORRESOURCE Null pointer or resource error.
 */
e2prom_status_t bsp_e2prom_handler_inst(
    bsp_at24c512_handler_t *handler_instance,
    e2prom_input_arg_t *input_arg)
{
    e2prom_status_t ret = E2PROM_OK;

    // log_d("bsp_e2prom_handler_inst start");

    if (NULL == handler_instance)
    {
        log_e("handler_instance is NULL");
        return E2PROM_ERRORRESOURCE;
    }

    if (NULL == input_arg)
    {
        log_e("input_arg is NULL");
        return E2PROM_ERRORRESOURCE;
    }

    if (NULL == input_arg->timebase_interface)
    {
        log_e("timebase_interface is NULL");
        return E2PROM_ERRORRESOURCE;
    }

    if (NULL == input_arg->iic_driver_interface)
    {
        log_e("iic_driver_interface is NULL");
        return E2PROM_ERRORRESOURCE;
    }

    if (NULL == input_arg->os_interface)
    {
        log_e("os_interface is NULL");
        return E2PROM_ERRORRESOURCE;
    }

    // Assign input interfaces to handler instance
    // Better : Void * Varibale passed to low level driver
    handler_instance->timebase_interface = input_arg->timebase_interface;
    handler_instance->iic_driver_interface = input_arg->iic_driver_interface;
    handler_instance->yield_interface = input_arg->yield_interface;
    handler_instance->os_interface = input_arg->os_interface;

    // Initialize the handler
    ret = bsp_at24c512_handler_init(handler_instance);
    if (E2PROM_OK != ret)
    {
        return E2PROM_ERRORRESOURCE;
    }

    // log_d("bsp_e2prom_handler_inst end");
    return E2PROM_OK;
}

/**;
 * @brief Fetch at24c512 values.
 *
 * @param[in]  e2prom_event_t.
 *
 * @retval E2PROM_OK   Data fetched successfully.
 * @retval E2PROM_ERROR Error in fetching data.
 */
static e2prom_status_t e2prom_operate_data(e2prom_event_t *event)
{
    e2prom_status_t ret = E2PROM_OK;

    switch (event->type)
    {
    case E2PROM_WRITE:
        ret = gp_at24c512_instance->pat24c512_instance->pf_write_data(
            gp_at24c512_instance->pat24c512_instance,
            event->memAddr, event->data, event->len);
        break;

    case E2PROM_READ:
        ret = gp_at24c512_instance->pat24c512_instance->pf_read_data(
            gp_at24c512_instance->pat24c512_instance,
            event->memAddr, event->data, event->len);
        break;

    default:
        ret = E2PROM_ERROR;
        break;
    }
    return ret;
}

/**
 * @brief Handler thread for processing E2PROM events.
 *
 * @param[in] argument Pointer to input arguments (input_arg).
 */
void e2prom_handler_thread(void *argument)
{
    e2prom_input_arg_t *input_arg = NULL;
    e2prom_event_t *e2prom_event = NULL;
    e2prom_status_t ret = E2PROM_OK;
    gp_at24c512_instance = malloc(sizeof(bsp_at24c512_handler_t));
    // AT24C512 instance
    bsp_at24c512_driver_t bsp_at24c512_driver;

    if (NULL == argument)
    {
        log_e("e2prom_handler_thread argument is NULL");
        return;
    }

    input_arg = (e2prom_input_arg_t *)argument;

    gp_at24c512_instance->pat24c512_instance = &bsp_at24c512_driver;

    ret = bsp_e2prom_handler_inst(
        gp_at24c512_instance,
        input_arg);
    for (;;)
    {
        // 1. Read event from the queue
        ret = gp_at24c512_instance->os_interface->os_queue_get(
            gp_at24c512_instance->event_queue_handle,
            &e2prom_event,
            MY_MAX_DELAY);
        // log_d("event mode = %d\r\n",e2prom_event->type);
        ret = e2prom_operate_data(e2prom_event);
        if (E2PROM_OK == ret)
        {
            // Data fetched successfully
            // log_d("e2prom operater success\r\n", ret);
        }
        else
        {
            // Data fetch error
            // log_d("e2prom operater fail ret = %d\r\n", ret);
        }
        vPortFree(e2prom_event);
    }
}
e2prom_status_t bsp_e2prom_write(uint16_t memAddr, uint8_t *data, uint16_t len)
{
    e2prom_status_t ret = E2PROM_OK;

    e2prom_event_t *event = (e2prom_event_t *)pvPortMalloc(sizeof(e2prom_event_t));
    event->data = data;
    event->len = len;
    event->memAddr = memAddr;
    event->type = E2PROM_WRITE;

    if (NULL == gp_at24c512_instance)
    {
        log_e("gp_at24c512_instance not mounted\r\n ");
        return E2PROM_ERRORRESOURCE;
    }
    if (NULL == gp_at24c512_instance->event_queue_handle)
    {
        log_e("gp_at24c512_instance->event_queue_handle failed\r\n ");
        return E2PROM_ERRORRESOURCE;
    }
    ret = gp_at24c512_instance->os_interface->os_queue_put(
        gp_at24c512_instance->event_queue_handle,
        &event, MY_MAX_DELAY);

    return ret;
}
e2prom_status_t bsp_e2prom_read(uint16_t memAddr, uint8_t *buffer, uint16_t len)
{
    e2prom_status_t ret = E2PROM_OK;

    e2prom_event_t *event = (e2prom_event_t *)pvPortMalloc(sizeof(e2prom_event_t));
    if (event == NULL)
    {
        ret = E2PROM_ERRORNOMEMORY;
        return ret;
    }
    event->data = buffer;
    event->len = len;
    event->memAddr = memAddr;
    event->type = E2PROM_READ;

    if (NULL == gp_at24c512_instance)
    {
        log_e("gp_at24c512_instance not mounted\r\n ");
        return E2PROM_ERRORRESOURCE;
    }
    if (NULL == gp_at24c512_instance->event_queue_handle)
    {
        log_e("gp_at24c512_instance->event_queue_handle failed\r\n ");
        return E2PROM_ERRORRESOURCE;
    }
    ret = gp_at24c512_instance->os_interface->os_queue_put(
        gp_at24c512_instance->event_queue_handle,
        &event, MY_MAX_DELAY);

    return ret;
}
uint8_t writeData[256], readData[256];
void Test_EEPROM(void)
{
    static uint32_t addr = 0;
    uint16_t i;

    for (i = 0; i < 256; i++)
        writeData[i] = (i + 1) % 256;

    if (bsp_e2prom_write(0x0000 + addr, writeData, 256) == E2PROM_OK)
    {
        log_d("write ok\r\n");
    }
    else
    {
        log_e("write error\r\n");
    }

    if (bsp_e2prom_read(0x0000 + addr, readData, 256) == E2PROM_OK)
    {
        for (i = 0; i < 256; i++)
        {
            if (readData[i] != writeData[i])
            {
                log_d("error test error data = [%d] = %d\r\n", i, readData[i]);
                break;
            }
        }
        if (i == 256)
        {
            log_d("Read verification passed!\n\r");
        }
    }
    memset(readData, 0, 256);
    addr += 256;
}

//******************************** Functions ********************************//
