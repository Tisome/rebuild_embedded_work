#include "at24cxx_handler.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "elog.h"

#define E2PROM_EVENT_QUEUE_LEN 16U
#define E2PROM_MAX_DELAY       0xFFFFFFFFUL

static e2prom_handler_t *g_e2prom_instance = NULL;

static e2prom_status_t e2prom_status_from_at24(at24cxx_status_t status)
{
    switch (status)
    {
    case AT24CXX_OK:
        return E2PROM_OK;
    case AT24CXX_BUSY:
        return E2PROM_BUSY;
    case AT24CXX_TIMEOUT:
        return E2PROM_TIMEOUT;
    case AT24CXX_ERROR_RESOURCE:
        return E2PROM_ERROR_RESOURCE;
    case AT24CXX_ERROR_PARAMETER:
        return E2PROM_ERROR_PARAMETER;
    case AT24CXX_ERROR_NO_MEMORY:
        return E2PROM_ERROR_NO_MEMORY;
    default:
        return E2PROM_ERROR;
    }
}

at24cxx_status_t at24cxx_handler_inst(at24cxx_handler_t *instance)
{
    if ((instance == NULL) || (instance->p_iic_driver_instance == NULL) ||
        (instance->p_at24cxx_dev_info == NULL) ||
        (instance->p_at24cxx_driver_instance == NULL))
    {
        return AT24CXX_ERROR_RESOURCE;
    }

    return AT24CXX_OK;
}

e2prom_status_t e2prom_handler_inst(e2prom_handler_t *instance,
                                    e2prom_input_arg_t *arg)
{
    if ((instance == NULL) || (arg == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if ((instance->at24cxx_handler_instance == NULL) ||
        (instance->os_interface_instance == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if ((arg->at24cxx_driver_instance == NULL) || (arg->dev_info_instance == NULL) ||
        (arg->iic_driver_instance == NULL) || (arg->os_interface_instance == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    instance->at24cxx_handler_instance->p_iic_driver_instance = arg->iic_driver_instance;
    instance->at24cxx_handler_instance->p_at24cxx_dev_info = arg->dev_info_instance;
    instance->at24cxx_handler_instance->p_at24cxx_driver_instance = arg->at24cxx_driver_instance;
    instance->os_interface_instance = arg->os_interface_instance;

    if (instance->os_interface_instance->os_queue_create == NULL)
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if (instance->event_queue_handle == NULL)
    {
        e2prom_status_t qret = instance->os_interface_instance->os_queue_create(
            E2PROM_EVENT_QUEUE_LEN,
            sizeof(e2prom_event_t *),
            &instance->event_queue_handle);
        if (qret != E2PROM_OK)
        {
            return qret;
        }
    }

    g_e2prom_instance = instance;

    return E2PROM_OK;
}

static e2prom_status_t e2prom_operate_event(const e2prom_event_t *event)
{
    at24cxx_handler_t *h = g_e2prom_instance->at24cxx_handler_instance;
    at24cxx_driver_t *drv = h->p_at24cxx_driver_instance;
    iic_driver_t *iic = h->p_iic_driver_instance;

    if ((event == NULL) || (drv == NULL) || (iic == NULL) || (iic->hi2c == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    at24cxx_status_t ret = AT24CXX_ERROR;
    if (event->event == E2PROM_WRITE)
    {
        ret = drv->pf_write(h->p_at24cxx_dev_info,
                            iic->hi2c,
                            event->mem_addr,
                            event->buf,
                            event->len);
    }
    else
    {
        ret = drv->pf_read(h->p_at24cxx_dev_info,
                           iic->hi2c,
                           event->mem_addr,
                           event->buf,
                           event->len);
    }

    return e2prom_status_from_at24(ret);
}

void task_e2prom_handler(void *arg)
{
    e2prom_input_arg_t *input_arg = (e2prom_input_arg_t *)arg;
    e2prom_event_t *event = NULL;

    static e2prom_handler_t e2prom;
    static at24cxx_handler_t at24cxx_handler;

    e2prom.at24cxx_handler_instance = &at24cxx_handler;
    e2prom.os_interface_instance = input_arg->os_interface_instance;
    e2prom.event_queue_handle = NULL;

    if (e2prom_handler_inst(&e2prom, input_arg) != E2PROM_OK)
    {
        log_e("e2prom_handler_inst failed");
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        e2prom_status_t ret = g_e2prom_instance->os_interface_instance->os_queue_take(
            g_e2prom_instance->event_queue_handle,
            &event,
            E2PROM_MAX_DELAY);
        if ((ret != E2PROM_OK) || (event == NULL))
        {
            continue;
        }

        (void)e2prom_operate_event(event);
        vPortFree(event);
    }
}

e2prom_status_t e2prom_write_async(uint16_t mem_addr, uint8_t *buf, uint16_t len)
{
    if ((g_e2prom_instance == NULL) || (g_e2prom_instance->event_queue_handle == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    e2prom_event_t *event = pvPortMalloc(sizeof(e2prom_event_t));
    if (event == NULL)
    {
        return E2PROM_ERROR_NO_MEMORY;
    }

    event->mem_addr = mem_addr;
    event->buf = buf;
    event->len = len;
    event->event = E2PROM_WRITE;

    e2prom_status_t ret = g_e2prom_instance->os_interface_instance->os_queue_give(
        g_e2prom_instance->event_queue_handle,
        &event,
        E2PROM_MAX_DELAY);
    if (ret != E2PROM_OK)
    {
        vPortFree(event);
    }

    return ret;
}

e2prom_status_t e2prom_read_async(uint16_t mem_addr, uint8_t *buf, uint16_t len)
{
    if ((g_e2prom_instance == NULL) || (g_e2prom_instance->event_queue_handle == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    e2prom_event_t *event = pvPortMalloc(sizeof(e2prom_event_t));
    if (event == NULL)
    {
        return E2PROM_ERROR_NO_MEMORY;
    }

    event->mem_addr = mem_addr;
    event->buf = buf;
    event->len = len;
    event->event = E2PROM_READ;

    e2prom_status_t ret = g_e2prom_instance->os_interface_instance->os_queue_give(
        g_e2prom_instance->event_queue_handle,
        &event,
        E2PROM_MAX_DELAY);
    if (ret != E2PROM_OK)
    {
        vPortFree(event);
    }

    return ret;
}
