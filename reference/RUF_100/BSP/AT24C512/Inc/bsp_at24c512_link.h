#ifndef __AT24C512_LINK
#define __AT24C512_LINK

#include "main.h"
#include "i2c.h"
#include "bsp_at24c512_handler.h"
#include "task.h"
#include "FreeRTOS.h"
#include "queue.h"

static yield_interface_t yield_interface =
{
    .pf_rtos_yield       = vTaskDelay
};

static timebase_interface_t timebase_interface =
{
    .pf_get_tick_count   = get_delay_tick
};

static at24c512_status_t iic_init_e2prom(void * parameter)
{
    at24c512_status_t ret = AT24C512_OK;
    e2prom_i2c_config();
    
    return ret;
}
static at24c512_status_t iic_deinit_e2prom (void *parameter)
{
        at24c512_status_t ret = AT24C512_OK;
        return ret;
}

static e2prom_status_t os_queue_create_e2prom(uint32_t item_num, uint32_t item_size, void **queue_handle)
{
    *queue_handle = xQueueCreate(item_num, item_size);
    if(NULL == *queue_handle)
    {
        return E2PROM_ERRORPARAMETER;
    } else {
        return E2PROM_OK;
    }
}
static e2prom_status_t os_queue_put_e2prom(void *queue_handle, void *item, uint32_t timeout)
{
    if (pdPASS == xQueueSend(queue_handle, item, timeout))
    {
        return E2PROM_OK;
    }
    else
    {
        return E2PROM_RESERVED;
    }
}

static e2prom_status_t os_queue_get_e2prom(void *queue_handle, void *msg, uint32_t timeout)
{
    if (pdPASS == xQueueReceive(queue_handle, msg, timeout))
    {
        return E2PROM_OK;
    }
    else
    {
        return E2PROM_RESERVED;
    }
}

#endif 
